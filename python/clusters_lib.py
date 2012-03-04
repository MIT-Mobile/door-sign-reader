import math

class Cluster:
    def __init__(self, region):
        self.regions = [region]
        self.minX = region.minX
        self.minY = region.minY
        self.maxX = region.maxX
        self.maxY = region.maxY
        self.minSize = region.size()
        self.maxSize = region.size()

    def addRegion(self, region):
        self.regions.append(region)
        self.addBox(region)

    def addCluster(self, cluster):
        self.regions += cluster.regions
        self.addBox(cluster)

    def addBox(self, box):
        if (box.minX < self.minX):
            self.minX = box.minX
        if (box.minY < self.minY):
            self.minY = box.minY
        if (box.maxX > self.maxX):
            self.maxX = box.maxX
        if (box.maxY > self.maxY):
            self.maxY = box.maxY
    
    # a non euclidean distance
    # which gives a simple estimate
    # of how close a region is to this cluster
    def distance(self, region):
        distance = 0.0
        if region.minX > self.maxX:
            distance += (region.minX - self.maxX)
        if region.maxX < self.minX:
            distance += (self.minX - region.maxX)
        if region.minY > self.maxY:
            distance += (region.minY - self.maxY)
        if region.maxY < self.minY:
            distance += (self.minY - region.maxY)
        return distance

    def alignment(self, region):
        height = 1. * min(region.height(), self.maxY-self.minY)
        topDiff = (region.maxY-self.maxY) / height
        bottomDiff = (region.minY-self.minY) / height
        return math.sqrt(topDiff*topDiff + bottomDiff*bottomDiff)

    def averageArea(self):
        totalAccumulator = 0
        for region in self.regions:
            totalAccumulator += region.area()
        return 1. * totalAccumulator / len(self.regions)

    def width(self):
        return 1.0 * self.maxX - self.minX

    def height(self):
        return 1.0 * self.maxY - self.minY

    def yChiSquare(self):
        total = 0.0
        for region in self.regions:
            diff = 1. * region.center()[1] - ((self.minY + self.maxY) / 2)
            diff = diff / (self.maxY - self.minY)
            total += diff * diff
        total = total / len(self.regions)
        return total

closenessFactor = 2.0
alignmentFactor = 0.2
areaFactor = 5.0

def findClusters(regions):
    
    clusters = []
    for region in regions:
        if region.height() == 0:
            continue

        nearbyClusters = []
        farClusters = []
        for cluster in clusters:
            if (cluster.alignment(region) < alignmentFactor and
                cluster.distance(region) < closenessFactor * region.height() and
                cluster.averageArea() / areaFactor <= region.area() and
                cluster.averageArea() * areaFactor >= region.area()):

                nearbyClusters.append(cluster)
            else:
                farClusters.append(cluster)

        newCluster = Cluster(region)
        if len(nearbyClusters) > 0:
            for nearbyCluster in nearbyClusters:
                newCluster.addCluster(nearbyCluster)
        clusters = farClusters + [newCluster]

    return clusters
            
