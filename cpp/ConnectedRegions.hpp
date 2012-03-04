#include <inttypes.h>

#define MAX_PARTIAL_REGION_COUNT 1024
#define MAX_UNIQUE_REGION_COUNT 512

class ConnectedRegion {
    ConnectedRegion *linkedRegion;
    
    ConnectedRegion* getRootRegion();
    
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
    
    bool isRootRegion();
    
    uint16_t getRegionID();
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
    
    ConnectedRegions();
};
