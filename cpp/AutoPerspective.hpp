#include <opencv2/core/core.hpp>
#include "Lines.hpp"

bool areLinesCandidatesToComputePerspective(LineSets &lineSets);

// image and dest are a 1 channel 1 byte matrix
bool removePerspective(cv::Mat &image, cv::Mat &dest);

// compute a likely vanishing point for the set of lines
Homogeneous2dPoint vanishingPoint(list<Vec4i> &lines);