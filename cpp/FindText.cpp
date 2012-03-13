#include "AutoPerspective.hpp"
#include "ConnectedRegionsLib.hpp"
#include <opencv2/core/core.hpp>
#include <memory>
#include "FindText.hpp"

#define MAX_REGION_COUNT 75
#define REGIONS_PADDING 5

using namespace cv;
using namespace std;

bool regionContains(const RegionBounds &bounds, const RegionsCluster &cluster);

void reduce2oneByte(Mat &source, Mat &dest);

void findTextCandidates(Mat &image, Mat &dest, list<RegionBounds> &foundRegions) {
    Mat perspectiveCorrected(image.rows, image.cols, CV_8UC1);
    
    CorrectionType correctionType = removePerspective(image, perspectiveCorrected);
    
    if (correctionType != NO_CORRECTION) {
        
        Mat binary(image.rows, image.cols, CV_8UC1);
        threshold(perspectiveCorrected, binary, 128, 255, THRESH_OTSU);
        
        Mat labels(image.rows, image.cols, CV_16UC1);
        auto_ptr<ConnectedRegions>regions = findConnectedRegions(binary, labels);
        
        list<ConnectedRegion> sortedRegions;
        regions->getSortedRegions(sortedRegions, MAX_REGION_COUNT);
        
        list<RegionsCluster> clusters;
        findClusters(sortedRegions, clusters);
        clusters.sort();
        clusters.reverse();
        
        // add padding to regions
        list<RegionsCluster>::iterator clusterIter;
        for (clusterIter = clusters.begin(); clusterIter != clusters.end(); clusterIter++) {
	        clusterIter->addPadding(REGIONS_PADDING, dest.cols, dest.rows);
        }        
        //LOGD("%d clusters found", clusters.size());
        
        for (clusterIter = clusters.begin(); clusterIter != clusters.end(); clusterIter++) {
            RegionsCluster cluster = *clusterIter;
            // always filter out the whole image
            if (!cluster.isWholeImage(dest.cols, dest.rows)) {
                
                list<RegionBounds>::iterator boundsIter;
                bool regionFound = false;
                for (boundsIter = foundRegions.begin(); boundsIter != foundRegions.end(); boundsIter++) {
                    if (cluster.insideBounds(*boundsIter)) {
                        regionFound = true;
                        break;
                    }
                }
                
                if (!regionFound) {
                    foundRegions.push_back(cluster.bounds());
                }
            }
        }
           

        perspectiveCorrected.copyTo(dest);
        
        // Code to make debugging easier
        /*
        reduce2oneByte(labels, dest);
        list<RegionBounds>::iterator boundsIter;
        for (boundsIter = foundRegions.begin(); boundsIter != foundRegions.end(); boundsIter++) {
            RegionBounds bounds = *boundsIter;
            rectangle(dest, cvPoint(bounds.left, bounds.top), cvPoint(bounds.right, bounds.bottom), 0, 5);
        }
        */
    }
}

void reduce2oneByte(Mat &source, Mat &dest) {
    for (int row = 0; row < source.rows; row++) {
        for (int col = 0; col < source.cols; col++) {
            dest.at<uint8_t>(row, col) = (uint8_t)source.at<uint16_t>(row, col);
        }
    }
}