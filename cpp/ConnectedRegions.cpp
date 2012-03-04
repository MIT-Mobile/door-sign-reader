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

    
ConnectedRegion::ConnectedRegion(uint16_t id, int x, int y, int color) {
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
        ConnectedRegion* newRegion = new ConnectedRegion(nextID, x, y, color);
        regionsMap[nextID] = newRegion;
        nextID++;
        return newRegion;
}
    
RegionsList ConnectedRegions::getAllRegions() {
        if (rootRegions.size() == 0) {
            RegionsMap::iterator regionsIterator;
            for (regionsIterator = regionsMap.begin(); regionsIterator != regionsMap.end(); ++regionsIterator) {
                ConnectedRegion *region = regionsIterator->second;
                if (region->isRootRegion()) {
                    rootRegions.push_back(region);
                }
            }
            return rootRegions;
        } 
        return rootRegions;
}
    
ConnectedRegion* ConnectedRegions::getRegionByID(uint16_t id) {
        if (id != lastQueryID || lastQueryRegion == NULL) {
            lastQueryRegion = regionsMap[id];
            lastQueryID = id;
        } 
        return lastQueryRegion;
    }
    
ConnectedRegions::ConnectedRegions() {
        nextID = 0;
        lastQueryID = 0;
        lastQueryRegion = NULL;
}
    
ConnectedRegions::~ConnectedRegions() {
        RegionsMap::iterator regionsIterator;
        for (regionsIterator = regionsMap.begin(); regionsIterator != regionsMap.end(); ++regionsIterator) {
            delete regionsIterator->second;
        }        
}

    

