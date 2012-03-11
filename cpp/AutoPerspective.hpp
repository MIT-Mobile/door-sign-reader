#include <opencv2/core/core.hpp>
#include "Lines.hpp"

enum CorrectionType {
    NO_CORRECTION,
    X_CORRECTION,
    Y_CORRECTION,
    X_Y_CORRECTION,
};

bool areLinesCandidatesToComputePerspective(LineSets &lineSets);

// image and dest are a 1 channel 1 byte matrix
CorrectionType removePerspective(cv::Mat &image, cv::Mat &dest);

// compute a likely vanishing point for the set of lines
Homogeneous2dPoint vanishingPoint(list<Vec4i> &lines);