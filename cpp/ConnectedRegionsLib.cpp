#include "ConnectedRegionsLib.hpp"

using namespace std;
using namespace cv;


// Source MAT needs to be 1 byte 1 channel
// Destination MAT needs to be 1 byte 1 channel

auto_ptr<ConnectedRegions> partiallyLabel(Mat source, Mat dest);

void completelyLabel(ConnectedRegions *regions, Mat dest);

auto_ptr<ConnectedRegions> findConnectedRegions(Mat source, Mat dest) {
    auto_ptr<ConnectedRegions> regions = partiallyLabel(source, dest);
    completelyLabel(regions.get(), dest);
    return regions;
}
     
auto_ptr<ConnectedRegions> partiallyLabel(Mat source, Mat dest) {
    auto_ptr<ConnectedRegions> allRegions(new ConnectedRegions());
    
    
    for (int j=0; j < source.rows; j++) {
        for (int i=0; i < source.cols; i++) {
            uint8_t currentColor = source.at<uint8_t>(j, i);
            ConnectedRegion *currentRegion = NULL;
            
            if (j > 0 && i > 0) {
                if (currentColor == source.at<uint8_t>(j-1, i-1)) {
                    ConnectedRegion *adjacentRegion = allRegions->getRegionByID(dest.at<uint16_t>(j-1, i-1));
                    currentRegion = adjacentRegion;
                }   
            }
            
            if (i > 0) {
                if (currentColor == source.at<uint8_t>(j, i-1)) {
                    ConnectedRegion *adjacentRegion = allRegions->getRegionByID(dest.at<uint16_t>(j, i-1));
                    if (currentRegion == NULL) {
                        currentRegion = adjacentRegion;
                    } else {
                        currentRegion->addRegion(adjacentRegion);
                    }
                }   
            }
            
            if (j > 0) {
                if (currentColor == source.at<uint8_t>(j-1, i)) {
                    ConnectedRegion *adjacentRegion = allRegions->getRegionByID(dest.at<uint16_t>(j-1, i));
                    if (currentRegion == NULL) {
                        currentRegion = adjacentRegion;
                    } else {
                        currentRegion->addRegion(adjacentRegion);
                    }
                }   
            }
            
            if (currentRegion != NULL) {
                dest.at<uint16_t>(j, i) = currentRegion->id;
                currentRegion->addPoint(i, j);
            } else {
                ConnectedRegion *newRegion = allRegions->newRegion(i, j, currentColor);
                dest.at<uint16_t>(j, i) = newRegion->id;
            }
        }
    }
    
    return allRegions;
    
}

#define SIMPLE_HASH_256(a) 991 * (349 + a) % 256

void completelyLabel(ConnectedRegions *regions, Mat dest) {
    for (int j=0; j < dest.rows; j++) {
        for (int i=0; i < dest.cols; i++) {
            ConnectedRegion *region = regions->getRegionByID(dest.at<uint16_t>(j, i));
            uint16_t id = region->getRegionID();
            dest.at<uint16_t>(j, i) = SIMPLE_HASH_256(id);
        }
    }
}
