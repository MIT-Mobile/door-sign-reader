#include <opencv2/core/core.hpp>
#include <list>

using std::list;
using cv::Vec4i;
using cv::vector;

class LineSets {
  public:
    list<Vec4i> horizontalLines;
    list<Vec4i> verticalLines;
};

class Homogeneous2dPoint {
  public:
    void setPoint(float x, float y);
    void setPointAtInfinity(float x, float y);
    void rescale();
    bool atInfinity();
    
    float x;
    float y;
    float w;
};

// Equation of a line in the form:
// ax + by + c = 0 
class LineParameters {
  public:
    float a;
    float b;
    float c;
};

class Lines {
  public:
    const static int NO_POINT_FOUND = 0;
    const static int POINT_FOUND = 1;
    
    static float angle(Vec4i &line);
    
    static LineSets horizontalAndVertical(vector<Vec4i> &lines);
    
    static list<Homogeneous2dPoint> intersections(list<Vec4i> &lines);
    
    /*
     *  returns NO_POINT_FOUND or POINT_FOUND
     */
    static int intersection(Vec4i &line1, Vec4i &line2, Homogeneous2dPoint &point);
    
    static void intersections(list<Vec4i> &lines, list<Homogeneous2dPoint> &points);
    
    /* This isnt a precise notion of distance                                                                            
     * just finds the vertex with the smallest distance                                                                  
     * to the other line
     */
    static float lineSegmentDistance(Vec4i &line1, Vec4i &line2);
    
  private:
    static float distancePointToLine(CvPoint &point, Vec4i &line);
    
    static float lineParameters(Vec4i &line, LineParameters *params);    
};

