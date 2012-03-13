#include "AnalyzeRegions.hpp"

#define CLOSENESS_FACTOR 2.0
#define ALIGNMENT_FACTOR 0.2
#define AREA_FACTOR 5.0

#define CLUSTER_MIN_WIDTH_FACTOR 1.5
#define CLUSTER_MIN_BLOB_COUNT 3

using namespace std;

void filterLikelyTextCluster(list<RegionsCluster> &clusters);

void findClusters(list<ConnectedRegion> &regions, list<RegionsCluster> &clusters) {
    list<ConnectedRegion>::iterator regionsIter;
    
    for (regionsIter = regions.begin(); regionsIter != regions.end(); regionsIter++) {
        ConnectedRegion region = *regionsIter;
        if (region.height() == 0) {
            continue;
        }
        
        list<RegionsCluster> nearbyClusters;
        list<RegionsCluster> farClusters;
        
        list<RegionsCluster>::iterator clusterIter;
        for (clusterIter = clusters.begin(); clusterIter != clusters.end();) {
            RegionsCluster cluster = *clusterIter;

            if ( (cluster.alignment(region) < ALIGNMENT_FACTOR) &&
                 (cluster.distance(region) < CLOSENESS_FACTOR * region.height()) &&
                 (cluster.averageArea() / AREA_FACTOR <= region.area()) &&
		         (cluster.averageArea() * AREA_FACTOR >= region.area()) ) {
                    
                    nearbyClusters.push_front(cluster);
                    
                    // move out of the main list since it will be put
                    // into the new cluster we are creating
                    // note we dont need to increment the iterator
                    // because removing current item effectively moves us forward
                    clusterIter = clusters.erase(clusterIter);
            } else {
                    clusterIter++;
            }
        }
    
        RegionsCluster newCluster(region);
        if (nearbyClusters.size() > 0) {
            for (clusterIter = nearbyClusters.begin(); clusterIter != nearbyClusters.end(); clusterIter++) {
                newCluster.addCluster(*clusterIter);
            }
        }
        clusters.push_front(newCluster);
    }
    filterLikelyTextCluster(clusters);
}

void filterLikelyTextCluster(list<RegionsCluster> &clusters) {
    list<RegionsCluster>::iterator clusterIter;
    for (clusterIter = clusters.begin(); clusterIter != clusters.end();) {
        
        if (clusterIter->blobsCount() <= 1) {
            clusterIter = clusters.erase(clusterIter);
            
        } else if ( (clusterIter->blobsCount() <= CLUSTER_MIN_BLOB_COUNT) &&
                    (clusterIter->width() < (clusterIter->height() * CLUSTER_MIN_WIDTH_FACTOR)) ) {
                
                    clusterIter = clusters.erase(clusterIter);
        } else {
            clusterIter++;
        }
    }
}
