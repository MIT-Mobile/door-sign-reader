#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <memory>
#include "ConnectedRegions.hpp"

// Source MAT needs to be 1 byte 1 channel
// Destination MAT needs to be 1 byte 1 channel


std::auto_ptr<ConnectedRegions> findConnectedRegions(cv::Mat source, cv::Mat dest);
