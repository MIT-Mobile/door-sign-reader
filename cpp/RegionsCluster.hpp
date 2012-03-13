#include "ConnectedRegions.hpp"
#include <list>

struct RegionBounds {
    int top, bottom, left, right;
};

class RegionsCluster {

    int minX, maxX, minY, maxY;

    std::list<ConnectedRegion> regions;
    
    void addBox(int minX, int maxX, int minY, int maxY);
    
  public:

    RegionsCluster(ConnectedRegion &region);
    
    void addCluster(RegionsCluster &cluster);
    
    float distance(ConnectedRegion &region);
    
    float alignment(ConnectedRegion &region);
    
    float averageArea();
    
    float width();
    
    float height();
    
    float area();
    
    bool operator<(RegionsCluster other);
 
    RegionBounds bounds();

    void addPadding(int padding, int width, int height);
    
    bool isWholeImage(int width, int height);
    
    bool insideBounds(RegionBounds &bounds);
    
    int blobsCount();
};
