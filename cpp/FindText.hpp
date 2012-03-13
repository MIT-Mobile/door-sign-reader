#include <list>
#include <opencv2/core/core.hpp>
#include "AnalyzeRegions.hpp"

void findTextCandidates(cv::Mat &image, cv::Mat &textCandidates, std::list<RegionBounds> &foundRegions);
