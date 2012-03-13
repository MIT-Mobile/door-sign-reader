#include "ConnectedRegions.hpp"

// ConnectedRegion implementation

using namespace std;

ConnectedRegion* ConnectedRegion::getRootRegion() {
        if (linkedRegion == NULL) {
            return this;
        } else {
            ConnectedRegion *rootRegion = linkedRegion->getRootRegion();
            // this is an optimization to keep                                                                                       
            // give sets as few links as possible to the root     
            linkedRegion = rootRegion;
            return rootRegion;
        }
}

    
void ConnectedRegion::initialize(uint16_t id, int x, int y, int color) {
        linkedRegion = NULL;
        this->id = id;
        minX = x;
        maxX = x;
        minY = y;
        maxY = y;
        this->color = color;
}
    
void ConnectedRegion::addPoint(int x, int y) {
        if (isRootRegion()) {
            if (x < minX) {
                minX = x;
            } 
            if (x > maxX) {
                maxX = x;
            }
            if (y < minY) {
                minY = y;
            }
            if (y > maxY) {
                maxY = y;
            } 
        } else {
            getRootRegion()->addPoint(x, y);
        }
}
    
void ConnectedRegion::addRegion(ConnectedRegion *otherRegion) {
        ConnectedRegion *rootRegion = getRootRegion();
        if (rootRegion != otherRegion->getRootRegion()) {
            otherRegion->addPoint(rootRegion->minX, rootRegion->minY);
            otherRegion->addPoint(rootRegion->maxX, rootRegion->maxY);
            rootRegion->linkedRegion = otherRegion->getRootRegion();
        }
}
    
float ConnectedRegion::width() {
        return (maxX - minX);
}
    
float ConnectedRegion::height() {
        return (maxY - minY);
}

float ConnectedRegion::area() {
    return width() * height();
}

bool ConnectedRegion::isRootRegion() {
        return (linkedRegion == NULL);
}
    
bool ConnectedRegion::operator<(ConnectedRegion other) {
        return (area()) < (other.area());
}

uint16_t ConnectedRegion::getRegionID() {
        if (isRootRegion()) {
            return id;
        } else {
            return getRootRegion()->id;
        }
}


// ConnectedRegions implementation

ConnectedRegion* ConnectedRegions::newRegion(int x, int y, int color) {
        if (nextID == MAX_PARTIAL_REGION_COUNT) {
            nextID = MAX_PARTIAL_REGION_COUNT - 1;
        }
    
        ConnectedRegion* newRegion = &(allRegions[nextID]);
        newRegion->initialize(nextID, x, y, color);
        nextID++;
        return newRegion;
}
    
ConnectedRegion** ConnectedRegions::getUniqueRegions() {
        if (uniqueRegionsCount == 0) {
            for (int regionID=0; regionID < nextID; regionID++) {
                ConnectedRegion *region = &(allRegions[regionID]);
                if (region->isRootRegion()) {
                    uniqueRegions[uniqueRegionsCount] = region;
                    uniqueRegionsCount++;
                }
                if (uniqueRegionsCount == MAX_UNIQUE_REGION_COUNT) {
                    break;
                }
            }
        } 
        return uniqueRegions;
}

int ConnectedRegions::getRegionsCount() {
    if (uniqueRegionsCount == 0) {
        getUniqueRegions();
    }
    return uniqueRegionsCount;
}
    
ConnectedRegion* ConnectedRegions::getRegionByID(uint16_t id) {
    return &(allRegions[id]);
}
   
void ConnectedRegions::getSortedRegions(list<ConnectedRegion> &regions, int maxRegionsCount) {
    int regionsCount = getRegionsCount();
    for (int i = 0; i < regionsCount; i++) {
        regions.push_front(*uniqueRegions[i]);
    }
    regions.sort();
    
    if (maxRegionsCount < regions.size()) {
        list<ConnectedRegion>::iterator regionIter;
        int i = 0;
        
        for (regionIter = regions.begin(); regionIter != regions.end(); i++) {
            if (i <= maxRegionsCount) {
                regionIter++;
            } else {
                regionIter = regions.erase(regionIter);
            }
        }
    }
}

ConnectedRegions::ConnectedRegions() {
        nextID = 0;
        uniqueRegionsCount = 0;
}


    

