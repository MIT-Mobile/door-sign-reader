import cv
import disjoint_sets
import clusters_lib

class DisjointRegions(disjoint_sets.DisjointSets):
    def __init__(self):
        super(DisjointRegions, self).__init__()
        self.nextID = 0

    def createRegion(self, color, x, y):
        id = self.nextID
        self.nextID += 1
        self.elements[id] = self.Region(self, id, color, x, y)
        return self.elements[id]

    def getAllRegions(self):
        return self.getAllSets()

    class Region(disjoint_sets.DisjointSets.Set):

        def __init__(self, disjointRegions, id, color, x, y):
            super(DisjointRegions.Region, self).__init__(DisjointRegions)
            self.id = id
            self.color = color
            self.minX = x
            self.maxX = x
            self.minY = y
            self.maxY = y
            self.children = []

        def getRegionID(self):
            return self.getRootSet().id

        def getMinX(self):
            return self.getRootSet().minX

        def getMaxX(self):
            return self.getRootSet().maxX

        def getMinY(self):
            return self.getRootSet().minY

        def getMaxY(self):
            return self.getRootSet().maxY

        def addRegion(self, other):
            if self.getRootSet() != other.getRootSet():
                other.addPoint(self.getMinX(), self.getMinY())
                other.addPoint(self.getMaxX(), self.getMaxY())
                self.getRootSet().linkedSet = other.getRootSet()

        def addPoint(self, x, y):
            if self.isRootSet():
                if x < self.minX:
                    self.minX = x
                if x > self.maxX:
                    self.maxX = x
                if y < self.minY:
                    self.minY = y
                if y > self.maxY:
                    self.maxY = y
            else:
                self.getRootSet().addPoint(x, y)


        def area(self):
            return (self.getMaxX()-self.getMinX()) * (self.getMaxY()-self.getMinY())

        def center(self):
            return ((self.getMaxX()+self.getMinX())/2 , (self.getMaxY()+self.getMinY())/2)

        def size(self):
            return 1.0 * max(self.getMaxX()-self.getMinX(), self.getMaxY()-self.getMinY())

        def __repr__(self):
            return "bounding box = (%i, %i), (%i, %i)" % (self.minX, self.minY, self.maxX, self.maxY)


def partiallyLabel(src):

    def findAdjacentRegion(x, y):
        # adjacent region same color as this region
        if currentColor == src[y, x]:
            adjacentRegion = regionsGrid[y][x]
            if currentRegion != None:
                currentRegion.addRegion(adjacentRegion)
            return adjacentRegion

    regionsGrid = [[0 for col in range(src.cols)] for row in range(src.rows)]    
    allRegions = DisjointRegions()

    for y in range(src.rows):
        for x in range(src.cols):
            currentColor = src[y, x]
            currentRegion = None
            if (x > 0 and y > 0):
                currentRegion = findAdjacentRegion(x-1, y-1)

            if x > 0:
                currentRegion = findAdjacentRegion(x-1, y)

            if y > 0:
                currentRegion = findAdjacentRegion(x, y-1)

            if currentRegion:
               regionsGrid[y][x] = currentRegion
               currentRegion.addPoint(x, y)
            else:
               regionsGrid[y][x] = allRegions.createRegion(currentColor, x, y)

    return regionsGrid, allRegions

def simpleHash256(value):
    return 991 * (349 + value) % 256

def completeLabels(regionsGrid, dest):
    for y in range(dest.rows):
        for x in range(dest.cols):
            dest[y, x] = simpleHash256(7 * regionsGrid[y][x].getRegionID())

def drawRectangles(clusters, dest):
    for cluster in clusters:
        for region in cluster.regions:
            cv.Rectangle(dest, (region.minX, region.minY), (region.maxX, region.maxY), simpleHash256(2*region.id), 2, cv.CV_AA)
    
clusterMinSize = 3    
def filterClusters(clusters):
    maxChiSquare = 0.01
    filteredClusters = []
    for cluster in clusters:
        if len(cluster.regions) >= clusterMinSize:
            if cluster.yChiSquare() < maxChiSquare:
                filteredClusters.append(cluster)
    return filteredClusters

def labelImage(src, dest):
    regionsGrid, regions = partiallyLabel(src)

    completeLabels(regionsGrid, dest)
    allRegions = regions.getAllRegions()
    allRegions.sort(cmp=regionCmp, reverse=True)
    largestRegions = allRegions[0:75]

    clusters = clusters_lib.findClusters(largestRegions)
    clusters = filterClusters(clusters)
    #drawRectangles(clusters, dest)
    for region in clusters:
            cv.Rectangle(dest, (region.minX, region.minY), (region.maxX, region.maxY), simpleHash256(2*region.minY), 2, cv.CV_AA)

    return clusters

            
def imageMax(image):
    max = -1
    for y in range(image.rows):
        for x in range(image.cols):
            if image[y, x] > max:
                max = image[y, x]
    return max

def count(image):
    max = -1
    count = 0
    for y in range(image.rows):
        for x in range(image.cols):
            if image[y, x] > max:
                max = image[y, x]
                count += 1
    return count

def regionCmp(regionA, regionB):
    return cmp(regionA.size(), regionB.size())
