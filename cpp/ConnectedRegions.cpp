#include "ConnectedRegions.hpp"

// ConnectedRegion implementation

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
            if (y > maxX) {
                maxX = y;
            } 
        }
}
    
void ConnectedRegion::addRegion(ConnectedRegion *otherRegion) {
        if (getRootRegion() != otherRegion->getRootRegion()) {
            otherRegion->addPoint(minX, minY);
            otherRegion->addPoint(maxX, maxY);
            getRootRegion()->linkedRegion = otherRegion->getRootRegion();
        }
}
    
float ConnectedRegion::width() {
        return (maxX - minX);
}
    
float ConnectedRegion::height() {
        return (maxY - minY);
}

bool ConnectedRegion::isRootRegion() {
        return (linkedRegion == NULL);
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
    
ConnectedRegions::ConnectedRegions() {
        nextID = 0;
        uniqueRegionsCount = 0;
}

    

