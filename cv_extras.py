import cv

class Label:
    def __init__(self, id, color, x, y):
        self.id = id
        self.lowerLinkLabel = None
        self.color = color
        self.minX = x
        self.maxX = x
        self.minY = y
        self.maxY = y

    def isRoot(self):
        return (self.lowerLinkLabel == None)

    def getMinimumLabel(self):
        if self.isRoot():
            return self
        else:
            label = self.lowerLinkLabel.getMinimumLabel()
            self.lowerLinkLabel = label
            return label

    def getMinimumID(self):
        return self.getMinimumLabel().id

    def setLowerLinkLabel(self, other):
        other.addPoint(self.minX, self.minY)
        other.addPoint(self.maxX, self.maxY)
        self.lowerLinkLabel = other
 
    def linkLabel(self, other):
        if self.getMinimumID() > other.getMinimumID():
           self.getMinimumLabel().setLowerLinkLabel(other)
        elif self.getMinimumID() < other.getMinimumID():
           other.getMinimumLabel().setLowerLinkLabel(self)

    def addPoint(self, x, y):
        if self.isRoot():
            if x < self.minX:
                self.minX = x
            if x > self.maxX:
                self.maxX = x
            if y < self.minY:
                self.minY = y
            if y > self.maxY:
                self.maxY = y
        else:
            self.lowerLinkLabel.addPoint(x, y)


def getMergedLabel(a, b):
    if a == None:
       return b
    else:
       a.linkLabel(b)
       return a.getMinimumLabel()

def partiallyLabel(src):
    nextLabel = 0
    labelsGrid = [[0 for col in range(src.cols)] for row in range(src.rows)]    
    allLabels = []

    for y in range(src.rows):
        for x in range(src.cols):
            currentColor = src[y, x]
            currentLabel = None
            if (x > 0 and y > 0):
                upperLeftColor = src[y-1, x-1]
                if upperLeftColor == currentColor:
                    label = labelsGrid[y-1][x-1]
                    currentLabel = getMergedLabel(currentLabel, label)

            if x > 0:
                leftColor = src[y, x-1]
                if leftColor == currentColor:
                    label = labelsGrid[y][x-1]
                    currentLabel = getMergedLabel(currentLabel, label)

            if y > 0:
                upperColor = src[y-1, x]
                if upperColor == currentColor:
                    label = labelsGrid[y-1][x]
                    currentLabel = getMergedLabel(currentLabel, label)

            if currentLabel:
               labelsGrid[y][x] = currentLabel
               currentLabel.addPoint(x, y)
            else:
               newLabel = Label(nextLabel, currentColor, x, y)
               labelsGrid[y][x] = newLabel
               allLabels.append(newLabel)
               nextLabel += 1

    return labelsGrid, allLabels

def simpleHash256(value):
    return 991 * (349 + value) % 256

def completeLabels(labelsGrid, dest):
    for y in range(dest.rows):
        for x in range(dest.cols):
            dest[y, x] = simpleHash256(labelsGrid[y][x].getMinimumID())

def drawRectangles(labels, dest):
    for label in labels:
        if label.isRoot():
            cv.Rectangle(dest, (label.minX, label.minY), (label.maxX, label.maxY), simpleHash256(2*label.id), 2, cv.CV_AA)
            
def labelImage(src, dest):
    labelsGrid, allLabels = partiallyLabel(src)
    completeLabels(labelsGrid, dest)
    drawRectangles(allLabels, dest)
            
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
