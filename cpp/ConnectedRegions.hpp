#include <map>
#include <list>
#include <inttypes.h>


class ConnectedRegion {
    ConnectedRegion *linkedRegion;
    
    ConnectedRegion* getRootRegion();
    
  public:
        
    uint16_t id;
    int minX, minY;
    int maxX, maxY;
    int color;
      
    ConnectedRegion(uint16_t id, int x, int y, int color); 
    
    void addPoint(int x, int y);    
    
    void addRegion(ConnectedRegion *otherRegion);
    
    float width();
    float height();
    
    bool isRootRegion();
    
    uint16_t getRegionID();
};

typedef std::list<ConnectedRegion*> RegionsList;
typedef std::map<uint16_t, ConnectedRegion*> RegionsMap;

class ConnectedRegions {
    
    uint16_t nextID;
    
    RegionsMap regionsMap;
    RegionsList rootRegions;
    
    uint16_t lastQueryID;
    ConnectedRegion *lastQueryRegion;

  public:
    ConnectedRegion* newRegion(int x, int y, int color);
    
    RegionsList getAllRegions();
    
    ConnectedRegion* getRegionByID(uint16_t id);
    
    ConnectedRegions();
    
    ~ConnectedRegions();
};
