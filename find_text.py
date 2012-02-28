#!/opt/local/bin/python2.6

import cv
import cv_extras
import vanishing_points
import sys
import math

originalImage = cv.LoadImage(sys.argv[1])
gray = cv.CreateMat(originalImage.height, originalImage.width, cv.CV_8UC1)
edges = cv.CreateMat(originalImage.height, originalImage.width, cv.CV_8UC1)
linesMat = cv.CreateMat(originalImage.height, originalImage.width, cv.CV_8UC1)
noPerspective = cv.CreateMat(originalImage.height, originalImage.width, cv.CV_8UC3)

cv.CvtColor(originalImage, gray, cv.CV_RGB2GRAY)

cv.Canny(gray, edges, 50, 100)
cv.SaveImage('edges.png', edges)

storage = cv.CreateMemStorage(512)

# look for lines with iterative adjustment of thresholds
def lineSetCmp(x, y):
    return cmp(len(x.lines), len(y.lines))

def findLineSets(lines):
    lineSets = vanishing_points.lineSets(lines)
    lineSets.sort(cmp=lineSetCmp, reverse=True)
    if len(lineSets) < 2:
        return None
    for lineSet in lineSets[0:2]:
        if len(lineSet.lines) < 2:
            return None
    return lineSets[0:2]

houghThreshold = 150
lines = cv.HoughLines2(edges, storage, cv.CV_HOUGH_PROBABILISTIC, 1, 0.005, houghThreshold, 60, 60)
lineSets = findLineSets(lines)
if lineSets == None:
    houghThreshold = 75
    lines = cv.HoughLines2(edges, storage, cv.CV_HOUGH_PROBABILISTIC, 1, 0.005, houghThreshold, 60, 60)
    lineSets = findLineSets(lines)

# draw lines
for line in lines:
    cv.Line(linesMat, line[0], line[1], 255, 1)
cv.SaveImage('lines.png', linesMat)




# try to figure out which set of lines is horizontal
# versus vertical
if abs(lineSets[0].angle() - math.pi/2) < math.pi/4:
    yLines = lineSets[0].lines
    xLines = lineSets[1].lines
else:
    yLines = lineSets[1].lines
    xLines = lineSets[0].lines


xIntersections = vanishing_points.intersections(xLines)
yIntersections = vanishing_points.intersections(yLines)

xVanishingPoint = vanishing_points.vanishingPoint(xLines)
yVanishingPoint = vanishing_points.vanishingPoint(yLines)

# keep the center of the image invariant
fixedPoint = (originalImage.width / 2, originalImage.height / 2)
perspectiveMat = vanishing_points.homographyMat(xVanishingPoint, yVanishingPoint, fixedPoint)
cv.WarpPerspective(originalImage, noPerspective, perspectiveMat, cv.CV_WARP_INVERSE_MAP)

cv.SaveImage('no_perspective.png', noPerspective)


grayNoPerspective = cv.CreateMat(originalImage.height, originalImage.width, cv.CV_8UC1)
binaryNoPerspective = cv.CreateMat(originalImage.height, originalImage.width, cv.CV_8UC1)
labeled = cv.CreateMat(originalImage.height, originalImage.width, cv.CV_8UC1)
cv.CvtColor(noPerspective, grayNoPerspective, cv.CV_RGB2GRAY)
cv.Threshold(grayNoPerspective, binaryNoPerspective, 128, 255, cv.CV_THRESH_OTSU)
cv.SaveImage('binary.png', binaryNoPerspective)

clusters = cv_extras.labelImage(binaryNoPerspective, labeled)
cv.SaveImage('labeled.png', labeled)

cluster_index = 0
for cluster in clusters:
    padding = 5
    left = max(cluster.minX-padding, 0)
    top = max(cluster.minY-padding, 0)
    right = min(cluster.maxX+padding, originalImage.width)
    bottom = min(cluster.maxY+padding, originalImage.height)

    rect = (left, top, 
             right - left, bottom - top)
    subMat = cv.GetSubRect(noPerspective, rect)
    cv.SaveImage("text_candidate_" + str(cluster_index) + ".png", subMat)
    cluster_index += 1




