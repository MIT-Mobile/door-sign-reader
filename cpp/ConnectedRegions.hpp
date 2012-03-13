#ifndef CLASS_CONNECTED_REGIONS
#define CLASS_CONNECTED_REGIONS

#include <inttypes.h>
#include <list>

#define MAX_PARTIAL_REGION_COUNT 1024
#define MAX_UNIQUE_REGION_COUNT 512

class ConnectedRegion {
    ConnectedRegion *linkedRegion;
    
  public:
        
    uint16_t id;
    int minX, minY;
    int maxX, maxY;
    int color;
      
    void initialize(uint16_t id, int x, int y, int color); 
    
    void addPoint(int x, int y);    
    
    void addRegion(ConnectedRegion *otherRegion);
    
    float width();
    float height();
    float area();
    
    bool isRootRegion();
    
    uint16_t getRegionID();

    ConnectedRegion* getRootRegion();

    bool operator<(ConnectedRegion other);
};

class ConnectedRegions {
    
    uint16_t nextID;
    
    ConnectedRegion allRegions[MAX_PARTIAL_REGION_COUNT];
    
    ConnectedRegion *uniqueRegions[MAX_UNIQUE_REGION_COUNT];
    int uniqueRegionsCount;

  public:
    ConnectedRegion* newRegion(int x, int y, int color);
    
    ConnectedRegion** getUniqueRegions();
    
    int getRegionsCount();
    
    ConnectedRegion* getRegionByID(uint16_t id);
    
    void getSortedRegions(std::list<ConnectedRegion> &regions, int maxRegionsCount);
    
    ConnectedRegions();
};

#endif
