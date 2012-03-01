import math
import cv


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
    # zero corresponds to parrelell lines
    
    denominatorError = abs(y4-y3) + abs(x2-x1) + abs(x4-x3) + abs(y2 - y1) 
    #print denominator, denominatorError
    if lineSegmentDistance(line1, line2) < 10:
        return None

    if abs(denominator) >= 2 *denominatorError:
    #if denominator != 0:
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

def medianPoint(points):
    pointCount = len(points)
    xValues = []
    yValues = []
    for point in points:
        xValues.append(point[0])
        yValues.append(point[1])
    xValues = sorted(xValues)
    yValues = sorted(yValues)
    if pointCount % 2 == 0:
        midpointUpper = pointCount / 2
        return ((xValues[midpointUpper] + xValues[midpointUpper - 1])/2, (yValues[midpointUpper] + yValues[midpointUpper - 1])/2)
    else:
        return (xValues[pointCount / 2], yValues[pointCount / 2])

def vanishingPoint(lines):
    vanishingPoints = intersections(lines)
    finitePoints = []
    pointsAtInfinity = []
    for point in vanishingPoints:
        if point[2] == 0:
            pointsAtInfinity.append(point)
        else:
            finitePoints.append(point)

    print len(finitePoints), len(pointsAtInfinity)
    if len(finitePoints) > 2 * len(pointsAtInfinity):
        point = medianPoint(finitePoints)
        scale = getPointScale(point)
        return (point[0] / scale, point[1] / scale, 1 / scale)
    else:
        point = medianPoint(pointsAtInfinity)
        return (point[0], point[1], 0)

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

