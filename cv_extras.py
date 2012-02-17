import cv

class Label:
    def __init__(self, id):
        self.id = id
        self.lowerLinkLabel = None

    def getMinimumLabel(self):
        if self.lowerLinkLabel == None:
            return self
        else:
            label = self.lowerLinkLabel.getMinimumLabel()
            self.lowerLinkLabel = label
            return label

    def getMinimumID(self):
        return self.getMinimumLabel().id

    def linkLabel(self, other):
        if self.getMinimumID() > other.getMinimumID():
           self.getMinimumLabel().lowerLinkLabel = other
        elif self.getMinimumID() < other.getMinimumID():
           other.getMinimumLabel().lowerLinkLabel = self

def getMergedLabel(a, b):
    if a == None:
       return b
    else:
       a.linkLabel(b)
       return a.getMinimumLabel()

def partiallyLabel(src):
    nextLabel = 0
    labelsGrid = [[0 for col in range(src.cols)] for row in range(src.rows)]    

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
            else:
               newLabel = Label(nextLabel)
               labelsGrid[y][x] = newLabel
               nextLabel += 1

    return labelsGrid

def simpleHash256(value):
    return 991 * (349 + value) % 256

def completeLabels(labelsGrid, dest):
    for y in range(dest.rows):
        for x in range(dest.cols):
            dest[y, x] = simpleHash256(labelsGrid[y][x].getMinimumID())

def labelImage(src, dest):
    labelsGrid = partiallyLabel(src)
    completeLabels(labelsGrid, dest)
            
def max(image):
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
