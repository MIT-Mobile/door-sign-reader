import cv
import disjoint_sets

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

        def contains(self, other):
            contains = (self.minX <= other.minX)
            contains = contains and (self.maxX >= other.maxX)
            contains = contains and (self.minY <= other.minY)
            contains = contains and (self.maxY >= other.maxY)
            return contains

        def isConnected(self):
            return len(self.children) == 0

        def isChildrenConnected(self):
            for child in self.children:
                if not child.isConnected():
                    return False
            return True

        def __repr__(self):
            return "bounding box = (%i, %i), (%i, %i) with %i children" % (self.minX, self.minY, self.maxX, self.maxY, len(self.children))

"""
# Labels which belong together
# i.e. labels that have roughly similar size and color
class SimilarLabels:

    def __init__(self, firstLabel):
        self.labels = [firstLabel]

    def addLabel(self, label):
        self.labels.append(label)

    def averageArea(self):
        total = 0
        for label in labels:
            total += label.area()
        return (1.0 * total / len(labels))

    # this method tries to predict
    # if passed in label belongs
    # with the labels already passed in
    def labelMatches(self, label):
        # first descriminate based on size
        sizeRatio = label.area() / self.averageArea()
        if sizeRatio > 3 or sizeRatio < (1./3):
           return false

        # only join objects of the same color
        if labels[0].color != label.color
           return false 
         
        return false       


    # Attempts to guess at a group of labels
    # which are aligned
    def alignedLabels(self):
        averageSlope = 0
        averageYIntercept = 0
        labelsCount = len(self.labels)
        for i in range(labelsCount)):
            for j in range(labelsCount):
                if i == j:
                    continue
                
                center1 = self.labels[i].center()
                center2 = self.labels[j].center()
                slope = 1. * (center2[1] - center1[1]) / (center2[0] - center1[0])
                yIntercept = center1[1] - slope * center1[0]

                averageSlope += slope
                averageYIntercept += yIntercept

        averageSlope = averageSlope / labelsCount / (labelsCount-1)
        averageYIntercept = averageYIntercept / labelsCount / (labelsCount-1)
"""

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

# recursive algorithm
# to create a Trea of all the regions
# returns a list of top level regions
def buildRegionsTree(regions):
    previousRegions = []
    for region in regions:
        currentRegions = []
        containerFound = False
        for previousRegion in previousRegions:
            if previousRegion.contains(region):
                previousRegion.children.append(region)
                containerFound = True
                break
            elif region.contains(previousRegion):
                region.children += [previousRegion] + previousRegion.children
                previousRegion.children = []
            else:
                currentRegions.append(previousRegion)
        if not containerFound:
            currentRegions.append(region)
            previousRegions = currentRegions

    for region in previousRegions:
        region.children = buildRegionsTree(region.children)

    return previousRegions

def simpleHash256(value):
    return 991 * (349 + value) % 256

def completeLabels(regionsGrid, dest):
    for y in range(dest.rows):
        for x in range(dest.cols):
            dest[y, x] = simpleHash256(regionsGrid[y][x].getRegionID())

def drawRectangles(regions, dest):
    for region in regions:
        if region.isChildrenConnected():
            cv.Rectangle(dest, (region.minX, region.minY), (region.maxX, region.maxY), simpleHash256(2*region.id), 2, cv.CV_AA)
            
def labelImage(src, dest):
    regionsGrid, regions = partiallyLabel(src)
    print len(regions.getAllRegions())
    completeLabels(regionsGrid, dest)
    buildRegionsTree(regions.getAllRegions())
    drawRectangles(regions.getAllRegions(), dest)
            
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
