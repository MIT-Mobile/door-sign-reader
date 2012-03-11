#include "AutoPerspective.hpp"
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

const double RADIUS_RESOLUTION = 1;        // in pixels
const double ANGLE_RESOLUTION = 0.05;     // in radians
const double MIN_LINE_LENGTH = 60;         // in pixels
const double MAX_LINE_GAP = 60;            // in pixels
const int POINTS_FOR_HOUGHP_LIMIT = 500;  

enum Axis {X_AXIS, Y_AXIS};

void filterVanishingLines(list<Vec4i> &lines, list<Vec4i> &vanishingLines, int width, int height);

bool vanishingPoint(list<Vec4i> &lines, Homogeneous2dPoint &point, int width, int height, Axis axis);

Homogeneous2dPoint medianPoint(list<Homogeneous2dPoint> &points, int centerX, int centerY, Axis axis);

float listMedian(list<float> &numbers);

void homographyMat(Homogeneous2dPoint &point1, Homogeneous2dPoint &point2, CvPoint fixed, Mat &perspectiveMat);

void drawLines(list<Vec4i> &lines, Mat &dest);

int arrayMax(int values[], int count); 

// used to limit the number of points
// the hough transform is computed on
int contentArea(Mat image);
void removePoints(Mat image, int removeFactor);



Axis branchPointAxis(Axis axis);
float thetaCoordinate(float x, float y);
CvPoint2D32f transformedPoint(float x, float y, Axis axis);

struct LinePair {
    
    // a pair of integers identify the two lines
    int i, j;
    
    float inverseDistance;
    float product;
    
    LinePair(int i, int j, float inverseDistance, float product) {
        this->i = i;
        this->j = j;
        this->inverseDistance = inverseDistance;
        this->product = product;
    }
    
    bool operator<(LinePair other) {
         return (inverseDistance < other.inverseDistance);
    } 
};

float inverseRadius(Homogeneous2dPoint &point, int centerX, int centerY);

CorrectionType removePerspective(cv::Mat &image, cv::Mat &dest) {
    timespec time1, time2, time3, time4, time5, time6;
    
    // edge detection    
    Mat edges(image.rows, image.cols, CV_8UC1);
    Canny(image, edges, 100, 50);
    
    // Cap the amount of data we use to look for lines
    // this primarily to prevent certain cases where
    // performances is really really slow
    int area = contentArea(edges);
    if (area > POINTS_FOR_HOUGHP_LIMIT) {
        int removeFactor = area / POINTS_FOR_HOUGHP_LIMIT;
        removeFactor = MAX(removeFactor, 2);
        removePoints(edges, removeFactor);
    }
    area = contentArea(edges);
    
    // attempt Hough Line detection with various thresholds
    CvMemStorage *linesStorages = cvCreateMemStorage(512);
    const int HOUGH_THRESHOLDS_COUNT = 2;
    int houghThresholds[] = {15, 10};
    
    // iteratively try to find lines, which can be used
    // to compute perspective
    LineSets lineSets;
    bool linesFound = false;
    vector<Vec4i> lines;
    for (int i = 0; i < HOUGH_THRESHOLDS_COUNT; i++) {

        HoughLinesP(edges, lines, RADIUS_RESOLUTION, ANGLE_RESOLUTION, houghThresholds[i], MIN_LINE_LENGTH, MAX_LINE_GAP);
        
        lineSets = Lines::horizontalAndVertical(lines);
        if (areLinesCandidatesToComputePerspective(lineSets)) {
            linesFound = true;
            break;
        }
    }    
    
    //LOGD("horizontal line count=%d, vertical line count=%d",lineSets.horizontalLines.size(), lineSets.verticalLines.size());
    
    // x and y vanishing points
    Homogeneous2dPoint vpX;
    Homogeneous2dPoint vpY;
    
    bool xPointFound = vanishingPoint(lineSets.horizontalLines, vpX, image.cols, image.rows, X_AXIS);
    bool yPointFound = vanishingPoint(lineSets.verticalLines, vpY, image.cols, image.rows, Y_AXIS);
    
    if (!xPointFound && !yPointFound) {
        //LOGD("no correction computable");
        return NO_CORRECTION;
    }
    
    
    // if only one point found, guess
    // the other point is simple a 90 degree rotation
    if (!xPointFound && yPointFound) {
        //LOGD("x vanishing point not found");
        vpX.setPointAtInfinity(vpY.y, -vpY.x); 
    }
    
    if (!yPointFound && xPointFound) {
        //LOGD("y vanishing point not found");
        vpY.setPointAtInfinity(-vpX.y, vpX.x); 
    }
    

    
    Mat map(3, 3, CV_32FC1);
    CvPoint fixedPoint = cvPoint(image.cols / 2, image.rows / 2);

    vpX.rescale();
    vpY.rescale();
    
    //LOGD("vpX=(%f, %f, %f)", vpX.x, vpX.y, vpY.w);
    //LOGD("vpY=(%f, %f, %f)", vpY.x, vpY.y, vpY.w);
    homographyMat(vpX, vpY, fixedPoint, map);
    
    // code meant to make debugging easier, by displaying
    // which lines this algorith used to compute corrections
    /*
    list<Vec4i> vanishingLines;
    filterVanishingLines(lineSets.horizontalLines, vanishingLines, image.cols, image.rows);
    filterVanishingLines(lineSets.verticalLines, vanishingLines, image.cols, image.rows);
    drawLines(vanishingLines, image);
    */
    
    warpPerspective(image, dest, map, dest.size(), CV_WARP_INVERSE_MAP);   
}

bool areLinesCandidatesToComputePerspective(LineSets &lineSets) {
    if (lineSets.horizontalLines.size() < 2) {
        return false;
    }
    if (lineSets.verticalLines.size() < 2) {
        return false;
    }
    return true;
}


void filterVanishingLinesHelper(
                          list<Vec4i> &lines, 
                          list<Vec4i> &vanishingLines,
                          int centerX, int centerY,
                          float inverseDistanceFactor,
                          float maxInverseDistance,
                          float intersectionLengthProduct,
                          float acceptanceSigma) {
    
 
    list<Vec4i>::iterator lineIter1;
    list<Vec4i>::iterator lineIter2;
    int i, j;
    list<LinePair> allLinePairs;
    
    for (lineIter1 = lines.begin(), i=0; lineIter1 != lines.end(); lineIter1++, i++) {
        lineIter2 = lineIter1;
        for (lineIter2++, j=i+1; lineIter2 != lines.end(); lineIter2++, j++) {
            
            Homogeneous2dPoint point;
            if (Lines::intersection(*lineIter1, *lineIter2, point)) {
                LinePair aLinePair(
                                   i,
                                   j,
                                   inverseRadius(point, centerX, centerY),
                                   Lines::length(*lineIter1) * Lines::length(*lineIter2));
                allLinePairs.push_front(aLinePair);
            }
        }
    }
    
    allLinePairs.sort();
    float totalLengthSquared = 0;
    float lastInverseDistance = 0;
    list<LinePair>::iterator linePairIter;
    for (linePairIter = allLinePairs.begin(); linePairIter != allLinePairs.end(); linePairIter++) {
        lastInverseDistance = linePairIter->inverseDistance;
        totalLengthSquared += linePairIter->product;
        if (totalLengthSquared > intersectionLengthProduct) {
            break;
        }
    }
    
    float inverseDistanceThreshold = inverseDistanceFactor * lastInverseDistance;
    inverseDistanceThreshold = MIN(inverseDistanceThreshold, maxInverseDistance);
    
    int intersectingLineCounts[lines.size()];
    memset(intersectingLineCounts, 0, lines.size() * sizeof(int));
    for(linePairIter = allLinePairs.begin(); linePairIter != allLinePairs.end(); linePairIter++) {
        if (linePairIter->inverseDistance <= inverseDistanceThreshold) {
            intersectingLineCounts[linePairIter->i] += 1;
            intersectingLineCounts[linePairIter->j] += 1;
        }
    }
    
    list<Vec4i>::iterator lineIter;
    int maxIntersectingLineCount = arrayMax(intersectingLineCounts, lines.size());
    
    for (i=0, lineIter = lines.begin(); i < lines.size(); i++, lineIter++) {
        if ((float)intersectingLineCounts[i] + sqrt(intersectingLineCounts[i]) > maxIntersectingLineCount) {
            vanishingLines.push_front(*lineIter);
        }
    }
}
        
void filterVanishingLines(list<Vec4i> &lines, list<Vec4i> &vanishingLines, int width, int height) {
    filterVanishingLinesHelper(
                         lines, vanishingLines, 
                         width/2, height/2,
                         1.2,
                         1. / 2. / MIN(width, height),
                         MIN(width, height) * MIN(width, height) * 3,
                         1.5);
}
                                 
bool vanishingPoint(list<Vec4i> &lines, Homogeneous2dPoint &point, int width, int height, Axis axis) {
    list<Vec4i> vanishingLines;
    filterVanishingLines(lines, vanishingLines, width, height);
    
    if (vanishingLines.size() == 0) {
        return false;
    }
    
    list<Homogeneous2dPoint> vanishingPoints;
    Lines::intersections(vanishingLines, vanishingPoints);
    
    if (vanishingPoints.size() != 0) {
        point = medianPoint(vanishingPoints, width / 2, height / 2, axis);
    } else {
        float deltaX = 0; 
        float deltaY = 0;
        list<Vec4i>::iterator vanishingLinesIter;
        for(vanishingLinesIter = vanishingLines.begin(); vanishingLinesIter != vanishingLines.end(); vanishingLinesIter++) {
            Vec4i line = *vanishingLinesIter;
            deltaX = deltaX + line[2] - line[0];
            deltaY = deltaY + line[3] - line[1];
            float norm = sqrt(deltaX*deltaX + deltaY*deltaY);
            point.setPointAtInfinity(deltaX / norm, deltaY / norm);
        }
    }    
    return true;
}

Homogeneous2dPoint medianPoint(list<Homogeneous2dPoint> &points, int centerX, int centerY, Axis axis) {
    axis = branchPointAxis(axis);
    
    int positivePoints = 0;
    int nonPositivePoints = 0;    
    list<Homogeneous2dPoint>::iterator pointsIter;
    for (pointsIter = points.begin(); pointsIter != points.end(); pointsIter++) {
        if (!pointsIter->atInfinity()) {
            CvPoint2D32f point = transformedPoint(pointsIter->x, pointsIter->y, axis); 
            if (point.y > 0) {
                positivePoints++;
            } else {
                nonPositivePoints++;
            }
        }
    }
    bool isPositive = (positivePoints > nonPositivePoints);
    
    list<float> inverseRadiusValues;
    list<float> thetaValues;
    for (pointsIter = points.begin(); pointsIter != points.end(); pointsIter++) {
        Homogeneous2dPoint aPoint = *pointsIter;
        
        inverseRadiusValues.push_front(inverseRadius(aPoint, centerX, centerY));

        if (!aPoint.atInfinity()) {
            aPoint.x = pointsIter->x - centerX;
            aPoint.y = pointsIter->y - centerY;
        }
        
        CvPoint2D32f point = transformedPoint(aPoint.x, aPoint.y, axis);        
        float theta = thetaCoordinate(point.x, point.y);
        if (isPositive && theta > M_PI) {
            theta = theta - M_PI;
        } else if (!isPositive && theta < M_PI) {
            theta = theta + M_PI;
        } 
        thetaValues.push_front(theta);
    }
    
    float inverseRadiusMedian = listMedian(inverseRadiusValues);
    float thetaMedian = listMedian(thetaValues);
    
    CvPoint2D32f point = transformedPoint(cos(thetaMedian), sin(thetaMedian), axis);
    
    Homogeneous2dPoint homogeneousPoint;
    if (inverseRadiusMedian == 0) {
        homogeneousPoint.setPointAtInfinity(point.x, point.y);
    } else {
        homogeneousPoint.setPoint(point.x/inverseRadiusMedian + centerX, point.y/inverseRadiusMedian + centerY);
    }
    return homogeneousPoint;
}

float listMedian(list<float> &numbers) {
    numbers.sort();
    int i = 0;
    
    list<float>::iterator numbersIter = numbers.begin();
    for (; i < (numbers.size() / 2); i++, numbersIter++) {
        // fast forward to the middle of list
    }
    
    if (numbers.size() % 2 == 0) {
        float upper = *numbersIter;
        numbersIter--;
        float lower = *numbersIter;
        return (upper + lower) / 2;
    } else {
        return *numbersIter;
    }
}
    
    
void homographyMat(Homogeneous2dPoint &point1, Homogeneous2dPoint &point2, CvPoint fixed, Mat &map) {
    map.at<float>(0, 0) = point1.x;
    map.at<float>(1, 0) = point1.y;
    map.at<float>(2, 0) = point1.w;
    
    map.at<float>(0, 1) = point2.x;
    map.at<float>(1, 1) = point2.y;
    map.at<float>(2, 1) = point2.w;
    
    map.at<float>(0, 2) = (float)fixed.x - (map.at<float>(0, 0) * fixed.x + map.at<float>(0, 1) * fixed.y);
    map.at<float>(1, 2) = (float)fixed.y - (map.at<float>(1, 0) * fixed.x + map.at<float>(1, 1) * fixed.y);
    map.at<float>(2, 2) = 1.0f - (map.at<float>(2, 0) * fixed.x + map.at<float>(2, 1) * fixed.y);    
}

void drawLines(list<Vec4i> &lines, Mat &dest) {
    list<Vec4i>::iterator lineIter;
    for (lineIter = lines.begin(); lineIter != lines.end(); lineIter++) {
        Vec4i aLine = (*lineIter);
        line(dest, cvPoint(aLine[0], aLine[1]), cvPoint(aLine[2], aLine[3]), 255, 5);
    }
}

int arrayMax(int values[], int count) {
    int i;
    int max = 0;
    for (i = 0; i < count; i++) {
        if (values[i] > max) {
            max = values[i];
        }
    }
    return max;
}

// pick the opposite axis, so the branch point
// is far away from the values we are looking at
Axis branchPointAxis(Axis axis) {
    if (axis == X_AXIS) {
        return Y_AXIS;
    } else if (axis == Y_AXIS) {
        return X_AXIS;
    } 
}

CvPoint2D32f transformedPoint(float x, float y, Axis axis) {
    if (axis == X_AXIS) {
        return cvPoint2D32f(x, y);
    } else if (axis == Y_AXIS) {
        return cvPoint2D32f(y, x);
    }
}

float thetaCoordinate(float x, float y) {
    // return angles between 0 and 2 PI
    // with 0 being the x-axis
    if (x == 0) {
        if (y > 0) {
            return M_PI / 2;
        } else {
            return M_PI * (3./2.);
        }
    } else {
        float angle = atan(abs(y/x));
        if (x >= 0 && y >= 0) {
            return angle;
        } else if (x >= 0 && y < 0) {
            return 2 * M_PI - angle;
        } else if (x < 0 && y >= 0) {
            return M_PI - angle;
        } else if (x < 0 && y < 0) {
            return M_PI + angle;
        }
    }
}

float inverseRadius(Homogeneous2dPoint &point, int centerX, int centerY) {
    if (!point.atInfinity()) {
        float deltaX = point.x - centerX;
        float deltaY = point.y - centerY;
        return 1. / sqrt(deltaX*deltaX + deltaY*deltaY);
    } else {
        return 0;
    }
}

int contentArea(Mat image) {
    int area = 0;
    for (int row = 0; row < image.rows; row++) {
        for (int col = 0; col < image.cols; col++) {
            if (image.at<uint8_t>(row, col) > 0) {
                area++;
            }
        }
    }
    return area;
}

void removePoints(Mat image, int removeFactor) {
    int pointCounter = 0;
    for (int row = 0; row < image.rows; row++) {
        for (int col = 0; col < image.cols; col++) {
            if (image.at<uint8_t>(row, col) > 0) {
                pointCounter++;
                pointCounter = pointCounter % removeFactor;
                if (pointCounter != 0) {
                    image.at<uint8_t>(row, col) = 0;
                }
            }
        }
    }
}


