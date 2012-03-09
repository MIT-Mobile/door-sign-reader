#!/opt/local/bin/python2.6

import cv
import cv_extras
import vanishing_points
import sys
import os
import math

outputDir = sys.argv[2]
if not os.path.exists(outputDir):
    os.mkdir(outputDir)

originalImage = cv.LoadImage(sys.argv[1])
gray = cv.CreateMat(originalImage.height, originalImage.width, cv.CV_8UC1)
edges = cv.CreateMat(originalImage.height, originalImage.width, cv.CV_8UC1)
linesMat = cv.CreateMat(originalImage.height, originalImage.width, cv.CV_8UC3)
noPerspective = cv.CreateMat(originalImage.height, originalImage.width, cv.CV_8UC3)

cv.CvtColor(originalImage, gray, cv.CV_RGB2GRAY)

cv.Canny(gray, edges, 50, 100)
cv.SaveImage(outputDir + '/edges.png', edges)

storage = cv.CreateMemStorage(512)

# look for lines with iterative adjustment of thresholds
def lineSetCmp(x, y):
    return cmp(len(x.lines), len(y.lines))

def findLineSets(lines):
    lineSets = vanishing_points.lineSets(lines)
    for lineSet in lineSets:
        if len(lineSet.lines) <= 2:
            return None
    return lineSets

houghThresholds = [150, 75, 35, 15, 7]
for threshold in houghThresholds:
    lines = cv.HoughLines2(edges, storage, cv.CV_HOUGH_PROBABILISTIC, 1, 0.005, threshold, 60, 60)
    lineSets = findLineSets(lines)
    if lineSets != None:
        break


# might consider using angle binning
# if an accurate perspective can't be found
#angles = [0.] * 100
#for line in lines:
#    if abs(vanishing_points.angle(line) - math.pi/2) < math.pi/4:
#        angle = vanishing_points.angle(line) - math.pi/2
#        if angle < 0:
#            angle += math.pi
#    else:
#        angle = vanishing_points.angle(line)
#    binnedAngle = int(math.floor(angle / math.pi * 100))
#    angles[binnedAngle] += 1
#print angles

xLines = lineSets[0].lines
yLines = lineSets[1].lines


# draw lines
for line in xLines:
    cv.Line(linesMat, line[0], line[1], (200, 100, 0), 3)
for line in yLines:
    cv.Line(linesMat, line[0], line[1], (0, 200, 100), 3)

xLines = vanishing_points.findVanishingLines(xLines, 720, 480)
yLines = vanishing_points.findVanishingLines(yLines, 720, 480)

for line in xLines:
    cv.Line(linesMat, line[0], line[1], (0, 50, 200), 1)
for line in yLines:
    cv.Line(linesMat, line[0], line[1], (200, 100, 0), 1)


cv.SaveImage(outputDir + '/lines.png', linesMat)

#exit(0)

#xIntersections = vanishing_points.intersections(xLines)
#yIntersections = vanishing_points.intersections(yLines)

xVanishingPoint = vanishing_points.vanishingPoint(xLines, (360, 240), 'x')
yVanishingPoint = vanishing_points.vanishingPoint(yLines, (360, 240), 'y')

#print xVanishingPoint, yVanishingPoint

# keep the center of the image invariant
fixedPoint = (originalImage.width / 2, originalImage.height / 2)
perspectiveMat = vanishing_points.homographyMat(xVanishingPoint, yVanishingPoint, fixedPoint)
cv.WarpPerspective(originalImage, noPerspective, perspectiveMat, cv.CV_WARP_INVERSE_MAP)
#cv.WarpPerspective(linesMat, noPerspective, perspectiveMat, cv.CV_WARP_INVERSE_MAP)

cv.SaveImage(outputDir + '/no_perspective.png', noPerspective)


grayNoPerspective = cv.CreateMat(originalImage.height, originalImage.width, cv.CV_8UC1)
binaryNoPerspective = cv.CreateMat(originalImage.height, originalImage.width, cv.CV_8UC1)
labeled = cv.CreateMat(originalImage.height, originalImage.width, cv.CV_8UC1)
cv.CvtColor(noPerspective, grayNoPerspective, cv.CV_RGB2GRAY)
cv.Threshold(grayNoPerspective, binaryNoPerspective, 128, 255, cv.CV_THRESH_OTSU)
cv.SaveImage(outputDir + '/binary.png', binaryNoPerspective)

clusters = cv_extras.labelImage(binaryNoPerspective, labeled)
cv.SaveImage(outputDir + '/labeled.png', labeled)

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
    cv.SaveImage(outputDir + "/text_candidate_" + str(cluster_index) + ".png", subMat)
    cluster_index += 1




