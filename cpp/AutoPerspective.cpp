#include "AutoPerspective.hpp"
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

const int HOUGH_THRESHOLDS_COUNT = 3;
const double RADIUS_RESOLUTION = 1;        // in pixels
const double ANGLE_RESOLUTION = 0.05;     // in radians
const double MIN_LINE_LENGTH = 60;         // in pixels
const double MAX_LINE_GAP = 60;            // in pixels

bool vanishingPoint(list<Vec4i> &lines, Homogeneous2dPoint &point);

Homogeneous2dPoint medianPoint(list<Homogeneous2dPoint> &points);

float listMedian(list<float> &numbers);

void homographyMat(Homogeneous2dPoint &point1, Homogeneous2dPoint &point2, CvPoint fixed, Mat &perspectiveMat);

void drawLines(LineSets &lines, Mat &dest);

bool removePerspective(cv::Mat &image, cv::Mat &dest) {
    timespec time1, time2, time3, time4, time5, time6;
    
    // edge detection
    Mat edges(image.rows, image.cols, CV_8UC1);
    Canny(image, edges, 50, 100);

    // attempt Hough Line detection with various thresholds
    CvMemStorage *linesStorages = cvCreateMemStorage(512);
    int houghThresholds[] = {150, 75, 35};
    
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
    
    if (!linesFound) {
        LOGD("no good set of lines found");
        return false;
    }
    
    Homogeneous2dPoint vp1;
    Homogeneous2dPoint vp2;
    
    if (!vanishingPoint(lineSets.horizontalLines, vp1)) {
        LOGD("vanishing point not found");
        return false;
    }
    if (!vanishingPoint(lineSets.verticalLines, vp2)) {
        LOGD("vanishing point not found");
        return false;
    }
    
    //LOGD("horizontal line count=%d, vertical line count=%d",lineSets.horizontalLines.size(), lineSets.verticalLines.size());
    
    Mat map(3, 3, CV_32FC1);
    CvPoint fixedPoint = cvPoint(image.cols / 2, image.rows / 2);

    homographyMat(vp1, vp2, fixedPoint, map);
    
    warpPerspective(image, dest, map, dest.size(), CV_WARP_INVERSE_MAP);   
    
    //drawLines(lineSets, dest);
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

// determins what fraction of points need to be
// at infinity, to decide the vanishing point is infinite
#define INFINITE_POINT_PREFERENCE_FACTOR 2

bool vanishingPoint(list<Vec4i> &lines, Homogeneous2dPoint &point) {
    list<Homogeneous2dPoint> vanishingPoints;
    Lines::intersections(lines, vanishingPoints);
    
    if (vanishingPoints.size() == 0) {
        return false;
    }
    
    list<Homogeneous2dPoint> finitePoints;
    list<Homogeneous2dPoint> infinitePoints;
    
    list<Homogeneous2dPoint>::iterator vanishingPointsIter;
    for (vanishingPointsIter = vanishingPoints.begin(); vanishingPointsIter != vanishingPoints.end(); vanishingPointsIter++) {

        if (vanishingPointsIter->atInfinity()) {
            infinitePoints.push_front(*vanishingPointsIter);
        } else {
            finitePoints.push_front(*vanishingPointsIter);
        }
    }
    
    if (finitePoints.size() > INFINITE_POINT_PREFERENCE_FACTOR * infinitePoints.size()) {
        point = medianPoint(finitePoints);
        point.rescale();
        
    } else {
        point = medianPoint(infinitePoints);
    }
    
    return true;
}

Homogeneous2dPoint medianPoint(list<Homogeneous2dPoint> &points) {
    list<float> xCoords;
    list<float> yCoords;
    list<float> wCoords;
    list<Homogeneous2dPoint>::iterator pointsIter;
    for (pointsIter = points.begin(); pointsIter != points.end(); pointsIter++) {
        xCoords.push_front(pointsIter->x);
        yCoords.push_front(pointsIter->y);
        wCoords.push_front(pointsIter->w);
    }
    Homogeneous2dPoint aMedianPoint;
    aMedianPoint.x = listMedian(xCoords);
    aMedianPoint.y = listMedian(yCoords);
    aMedianPoint.w = listMedian(wCoords);    
    return aMedianPoint;
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

void drawLines(LineSets &lines, Mat &dest) {
    list<Vec4i>::iterator lineIter;
    for (lineIter = lines.verticalLines.begin(); lineIter != lines.verticalLines.end(); lineIter++) {
        Vec4i aLine = (*lineIter);
        line(dest, cvPoint(aLine[0], aLine[1]), cvPoint(aLine[2], aLine[3]), 119, 2);
    }
    
    for (lineIter = lines.horizontalLines.begin(); lineIter != lines.horizontalLines.end(); lineIter++) {
        Vec4i aLine = (*lineIter);
        line(dest, cvPoint(aLine[0], aLine[1]), cvPoint(aLine[2], aLine[3]), 226, 2);
    }
}


