import math
import cv
import collections


def angle(line):
    ((x1, y1), (x2, y2)) = line
    if x2 - x1 != 0:
        angle = math.atan(1. * (y2 - y1) / (x2 - x1))
        if angle < 0:
            angle += math.pi 
        return angle
    else:
        return math.pi / 2


class LineSet:
    def __init__(self):
        self.lines = []

    def addLine(self, line):
        self.lines.append(line)

def lineSets(lines):
    xLines = LineSet()
    yLines = LineSet()
    lineSets = []
    for line in lines:
        # with in 45 degrees
        if abs(angle(line) - math.pi/2) < math.pi/4:
            yLines.addLine(line)
        else:
            xLines.addLine(line)
    return [xLines, yLines]


def loopOverPairs(items, pairCallback):
    itemCount = len(items)
    for i in range(itemCount):
        for j in range(itemCount - i - 1):
            item1 = items[i]
            item2 = items[i+j+1]
            pairCallback(item1, item2)


# attempts to filter out a set of lines
# which all vanish at the same point
def findVanishingLines(lines, width, height):

    # GIANT GUESSES!
    return findVanishingLinesHelper(
       lines, 
       center=(width / 2, height / 2), 
       inverseDistanceFactor = 2.0,
       maxInverseDistance = 1./ 3 / max(width, height),
       intersectingLengthProduct = min(width, height)**2 * 5,
       acceptanceSigma = 1.5)

def findVanishingLinesHelper(
      lines, 
      center, 
      inverseDistanceFactor, 
      maxInverseDistance, 
      intersectingLengthProduct, 
      acceptanceSigma):

    # attempt to filter out lines
    # that are much less parrelel than the rest 
    # of the lines

    lineCount = len(lines)
    LinePair = collections.namedtuple('LinePair','inverseDistance, product, i, j')
    linePairs = []
    def appendLinePair(i, j):
        if lineSegmentDistance(lines[i], lines[j]) > 30:
            intersectionPoint = intersection(lines[i], lines[j])
            inverseDistance = inverseRadius(intersectionPoint, center)
            product = length(lines[i]) * length(lines[j])
            aPair = LinePair(inverseDistance, product, i, j)
            linePairs.append(aPair)
    loopOverPairs(range(lineCount), appendLinePair)


    # find the fraction of pairs of lines that are most
    # similar
    def cmpLinePair(pair1, pair2):
        return cmp(pair1.inverseDistance, pair2.inverseDistance)
    linePairs.sort(cmpLinePair)


    totalLengthSquared = 0
    lastInverseDistance = 0
    for aPair in linePairs:
        lastInverseDistance = aPair.inverseDistance
        totalLengthSquared += aPair.product
        if totalLengthSquared > intersectingLengthProduct:
            break

    distanceThreshold = inverseDistanceFactor * lastInverseDistance
    distanceThreshold = min(distanceThreshold, maxInverseDistance)
 
    intersectingLineCounts = [0] * lineCount
    for aPair in linePairs:
        if aPair.inverseDistance <= distanceThreshold:
            intersectingLineCounts[aPair.i] += 1
            intersectingLineCounts[aPair.j] += 1

    # find lines which intersect enough other lines
    # at far distances to be considered a vanishing line
    vanishingLines = []
    maxIntersectingLineCount = max(intersectingLineCounts)

    for lineIndex in range(lineCount):
        count = intersectingLineCounts[lineIndex]
        if count + math.sqrt(count) >= maxIntersectingLineCount:
            vanishingLines.append(lines[lineIndex])

    return vanishingLines

def intersections(lines):

    points = []
    lineCount = len(lines)
    for i in range(lineCount):
        for j in range(lineCount - i - 1):
             line1 = lines[i]
             line2 = lines[i+j+1]
             point = intersection(line1, line2)
             if point:
                 points.append(point)

    return points

def intersection(line1, line2):

    ((x1, y1), (x2, y2)) = line1
    ((x3, y3), (x4, y4)) = line2

    denominator = (y4 - y3)*(x2 - x1) - (x4 - x3)*(y2 - y1)
    # zero corresponds to parrelel lines
    
    if lineSegmentDistance(line1, line2) < 10:
        return None

    if abs(denominator) > 0:
        numerator = ((x4 - x3)*(y1 - y3) - (y4 - y3)*(x1 - x3))
        u_a = 1. * numerator / denominator
        return (x1 + u_a * (x2 - x1), y1 + u_a * (y2 - y1), 1)
    else:
        deltaX = 1.0 * (x2 - x1)
        deltaY = 1.0 * (y2 - y1)
        # keep all angles near the positive x or positive y axis
        if abs(deltaX) > abs(deltaY):
            if deltaX < 0:
                deltaX = - deltaX
                deltaY = - deltaY
        else:
            if deltaY < 0:
                deltaX = - deltaX
                deltaY = - deltaY

        norm = math.sqrt(deltaX * deltaX + deltaY * deltaY)
        return (deltaX / norm, deltaY / norm, 0)

# this isnt a precise notion of distance
# just finds the vertex with the smallest distance
# to the other line
def lineSegmentDistance(line1, line2):
    (point1, point2) = line1
    (point3, point4) = line2

    lineParams1 = lineParameters(line1)
    lineParams2 = lineParameters(line2)
    distances = [
        distancePointToLine(point1, line2),
        distancePointToLine(point2, line2),
        distancePointToLine(point3, line1),
        distancePointToLine(point4, line1),
    ]
    return min(distances)

def distancePointToLine(point, line):
    (a, b, c) = lineParameters(line)
    return abs(a * point[0] + b * point[1] + c) / math.sqrt(a*a + b*b)

def inverseRadius(point, center):
    if point[2] > 0:
        return 1. / distancePointToPoint(point, center)
    else:
        return 0

def length(line):
    (point1, point2) = line
    return distancePointToPoint(point1, point2)

def distancePointToPoint(point1, point2):
    deltaX = point1[0] - point2[0]
    deltaY = point1[1] - point2[1]
    return math.sqrt(deltaX * deltaX + deltaY * deltaY)

# parameters of a line when given in the form
# ax + by + c = 0
def lineParameters(line):
    ((x1, y1), (x2, y2)) = line
    deltaX = x2 - x1
    deltaY = y2 - y1
    
    a = deltaY
    b = -deltaX
    c = -x1 * deltaY + y1 * deltaX
    return (a, b, c)

def getMedian(values):
  values = sorted(values)
  if len(values) % 2 == 1:
    return values[(len(values)+1)/2-1]
  else:
    lower = values[len(values)/2-1]
    upper = values[len(values)/2]
    return (float(lower + upper)) / 2  

        
def medianVanishingPoint(points, center, axis):       
    
    def thetaCoordinate(x, y):
        # return angles between 0 and 2pi,
        # with 0 being the x-axis
        if x == 0:
           if y > 0:
               return math.pi / 2
           else:
               return math.pi * (3./ 2)
        else:
            angle = math.atan(abs(y)/abs(x))
            if x >= 0 and y >= 0:
               return angle
            elif x >= 0 and y < 0:
               return 2 * math.pi - angle
            elif x < 0 and y >= 0:
               return math.pi - angle
            elif x < 0 and y < 0:
               return math.pi + angle
        
    pointCount = len(points)

    # since the branch point for theta
    # is the axis, we need to make sure
    # there is a mismatch between the given axis
    # and the x-axis
    if axis == 'x':
       xIndex = 1
       yIndex = 0
    elif axis == 'y':
       xIndex = 0
       yIndex = 1

    positivePoints = 0
    nonPositivePoints = 0
    for point in points:
        if point[2] != 0:
            if point[yIndex] > 0:
                positivePoints += 1        
            else:
                nonPositivePoints += 1

    isPositive = (positivePoints > nonPositivePoints)

    inverseRadiusValues = []
    thetaValues = []
    for point in points:
        inverseRadiusValues.append(inverseRadius(point, center))

        x = point[xIndex]
        y = point[yIndex]
        w = point[2]
        if w != 0:
            x -= center[xIndex]
            y -= center[yIndex]

        theta = thetaCoordinate(x, y)
        if isPositive and theta > math.pi:  
           theta -= math.pi
        elif not isPositive and theta < math.pi:
           theta += math.pi
        thetaValues.append(theta)

    inverseRadiusMedian = getMedian(inverseRadiusValues)
    thetaMedian = getMedian(thetaValues)
  
    x = math.cos(thetaMedian)
    y = math.sin(thetaMedian)

    if inverseRadiusMedian == 0:
        medianPoint = (x, y, 0)
    else:
        medianPoint = (x / inverseRadiusMedian + center[xIndex], y / inverseRadiusMedian + center[yIndex], 1)
 
    return (medianPoint[xIndex], medianPoint[yIndex], medianPoint[2])   

def vanishingPoint(lines, center, axis):
    vanishingPoints = intersections(lines)

    if vanishingPoints:
        point =  medianVanishingPoint(vanishingPoints, center, axis)
    else:
        (deltaX, deltaY) = (0.0, 0.0)
        for line in lines:
            deltaX += line[1][0] - line[0][0]
            deltaY += line[1][1] - line[0][1]
            norm = math.sqrt(deltaX**2 + deltaY**2)
            point = (deltaX / norm, deltaY / norm, 0)
            
    if point[2] > 0:
        scale = getPointScale(point)
        return (point[0] / scale, point[1] / scale, point[2] / scale)
    else:
        if abs(point[0]) > abs(point[1]) and point[0] < 0:
            point = (-point[0], -point[1], 0)
        elif abs(point[1]) > abs(point[0]) and point[1] < 0:
            point = (-point[0], -point[1], 0)
        return point

# vp1 - vanishing point 1
# vp2 - vanishing point 2
# fixed - a point that should be remain invariant
def homographyMat(vp1, vp2, fixed):
    mat = cv.CreateMat(3, 3, cv.CV_32FC1)

    mat[0, 0] = vp1[0]
    mat[1, 0] = vp1[1]
    mat[2, 0] = vp1[2]

    mat[0, 1] = vp2[0]
    mat[1, 1] = vp2[1]
    mat[2, 1] = vp2[2]

    mat[0, 2] = fixed[0] - (mat[0, 0] * fixed[0] + mat[0, 1] * fixed[1])
    mat[1, 2] = fixed[1] - (mat[1, 0] * fixed[0] + mat[1, 1] * fixed[1])
    mat[2, 2] = 1 - (mat[2, 0] * fixed[0] + mat[2, 1] * fixed[1])

    return mat

def getPointScale(point):

    if abs(point[0]) > abs(point[1]):
        return point[0]
    else:
        return point[1]

