import math
import cv


def angle(line):
    ((x1, y1), (x2, y2)) = line
    if x2 - x1 != 0:
        angle = math.atan(1. * (y2 - y1) / (x2 - x1))
        if angle < 0:
            angle = -angle
        return angle
    else:
        return math.pi / 2


class LineSet:
    def __init__(self, line):
        self.lines = [line]
        self.totalAngle = angle(line)

    def angle(self):
        return self.totalAngle / len(self.lines)

    def addLine(self, line):
        self.lines.append(line)
        self.totalAngle += angle(line)

def lineSets(lines):
    lineSets = []
    for line in lines:
        lineFound = False
        for lineSet in lineSets:
            # with in 0.3 radians
            if abs(angle(line) - lineSet.angle()) < 0.30:
                lineSet.addLine(line)
                lineFound = True
                break
        if not lineFound:
            lineSets.append(LineSet(line))

    return lineSets


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
    if denominator != 0:
        numerator = ((x4 - x3)*(y1 - y3) - (y4 - y3)*(x1 - x3))
        u_a = 1. * numerator / denominator
        return (x1 + u_a * (x2 - x1), y1 + u_a * (y2 - y1))
    else:
        return None

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
    if vanishingPoints:
        point = medianPoint(vanishingPoints)
        scale = getPointScale(point)
        return (point[0] / scale, point[1] / scale, 1 / scale)
    else:
        # lines dont intersect, vanishing point must be at infinity
        line = lines[0]
        deltaX = line[1][0] - line[0][0]
        deltaY = line[1][1] - line[0][1]
        norm = math.sqrt(deltaX * deltaX + deltaY * deltaY)
        return (deltaX / norm, deltaY / norm, 0)

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

