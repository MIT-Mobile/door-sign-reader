#include "Lines.hpp"
#include <math.h>

float Lines::angle(Vec4i &line) {
    int x1 = line[0];
    int y1 = line[1];
    int x2 = line[2];
    int y2 = line[3];
    
    if (x2 != x1) {
        float angle = atan((float)(y2 - y1)/(float)(x2 - x1));
        if (angle < 0) {
            angle += M_PI;
        }
        return angle;
    } else {
        return M_PI / 2;
    }
}

LineSets Lines::horizontalAndVertical(vector<Vec4i>& lines) {
    LineSets lineSets;
    for (int i=0; i < lines.size(); i++) {
        Vec4i line = lines[i];
        if (abs(angle(line) - M_PI/2) < M_PI/4) {
            lineSets.verticalLines.push_front(line);
        } else {
            lineSets.horizontalLines.push_front(line);
        }
    }
    return lineSets;
}

void Lines::intersections(list<Vec4i> &lines, list<Homogeneous2dPoint> &points) {
    list<Vec4i>::iterator linesIterator1;    
    
    for (linesIterator1 = lines.begin(); linesIterator1 != lines.end(); linesIterator1++) {
        
        // make sure not to look for a line intersecting itself.
        Vec4i line1 = *linesIterator1;
        list<Vec4i>::iterator linesIterator2 = linesIterator1;
        linesIterator2++;
        for (; linesIterator2 != lines.end(); linesIterator2++) {
            Vec4i line2 = *linesIterator2;
            Homogeneous2dPoint point;
            
            if (intersection(line1, line2, point) == POINT_FOUND) {
                points.push_front(point);
            }
        }
    }
}

int Lines::intersection(Vec4i &line1, Vec4i &line2, Homogeneous2dPoint &point) {
    int x1 = line1[0];
    int y1 = line1[1];
    int x2 = line1[2];
    int y2 = line1[3];
    
    int x3 = line2[0];
    int y3 = line2[1];
    int x4 = line2[2];
    int y4 = line2[3];
    
    // zero corresponds to parallel lines
    int denominator = (y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1);
    
    int denominatorError = abs(y4 - y3) + abs(x2 - x1) + abs(x4 - x3) + abs(y2 - y1);
    
    if (lineSegmentDistance(line1, line2) < 10) {
        return Lines::NO_POINT_FOUND;
    }
    
    const int CONFIDENCE_FACTOR = 2;
    if (abs(denominator) >= CONFIDENCE_FACTOR * denominatorError) {
        float numerator = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3));
        float u_a = numerator / denominator;
        point.setPoint(x1 + u_a * (x2 - x1), y1 + u_a * (y2 - y1));
    } else {
        float deltaX = x2 - x1;
        float deltaY = y2 - y1;
        
        // keep all angles near the positive x or positive y axis positive
        if (abs(deltaX) > abs(deltaY)) {
            if (deltaX < 0) {
                deltaX = -deltaX;
                deltaY = -deltaY;
            }
        } else {
            if (deltaY < 0) {
                deltaX = -deltaX;
                deltaY = -deltaY;
            }
        }

        float norm = sqrt(deltaX*deltaX + deltaY*deltaY);
        point.setPointAtInfinity(deltaX / norm, deltaY / norm);
    }
    
    return POINT_FOUND;    
}

float Lines::lineSegmentDistance(Vec4i &line1, Vec4i &line2) {
    CvPoint point1 = cvPoint(line1[0], line1[1]);
    CvPoint point2 = cvPoint(line1[2], line1[3]);
    CvPoint point3 = cvPoint(line2[0], line2[1]);
    CvPoint point4 = cvPoint(line2[2], line2[3]);   
    
    float dist1 = distancePointToLine(point1, line2);
    float dist2 = distancePointToLine(point2, line2);
    float dist3 = distancePointToLine(point3, line1);
    float dist4 = distancePointToLine(point4, line1);
    
    return MIN(dist1, MIN(dist2, MIN(dist3, dist4))); 
}

float Lines::distancePointToLine(CvPoint &point, Vec4i &line) {
    LineParameters lineParams;
    lineParameters(line, &lineParams);
    return abs(lineParams.a * point.x + lineParams.b * point.y + lineParams.c) / sqrt(lineParams.a * lineParams.a + lineParams.b * lineParams.b);
}

float Lines::lineParameters(Vec4i &line, LineParameters *params) {
    CvPoint point1 = cvPoint(line[0], line[1]);
    CvPoint point2 = cvPoint(line[2], line[3]);
    
    float deltaX = point2.x - point1.x;
    float deltaY = point2.y - point1.y;
    params->a = deltaY;
    params->b = -deltaX;
    params->c = -point1.x * deltaY + point1.y * deltaX;
}

// Homogenouse2dPoint implementation
void Homogeneous2dPoint::setPoint(float x, float y) {
    this->x = x;
    this->y = y;
    this->w = 1;
    
}

void Homogeneous2dPoint::setPointAtInfinity(float x, float y) {
    this->x = x;
    this->y = y;
    this->w = 0;
}

void Homogeneous2dPoint::rescale() {
    float scale;
    if (abs(x) > abs(y)) {
        scale = x;
    } else {
        scale = y;
    }

    x = x / scale;
    y = y / scale;
    w = w / scale;
}

bool Homogeneous2dPoint::atInfinity() {
    return (w == 0);
}

