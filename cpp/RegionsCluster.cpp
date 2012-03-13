#include "RegionsCluster.hpp"
#include <math.h>
#include <algorithm>

using namespace std;

RegionsCluster::RegionsCluster(ConnectedRegion &region) {
    regions.push_front(region);
    minX = region.minX;
    minY = region.minY;
    maxX = region.maxX;
    maxY = region.maxY;
}

void RegionsCluster::addCluster(RegionsCluster &cluster) {
    regions.splice(regions.end(), cluster.regions);
    addBox(cluster.minX, cluster.maxX, cluster.minY, cluster.maxY);
}

void RegionsCluster::addBox(int aMinX, int aMaxX, int aMinY, int aMaxY) {
    if (aMinX < minX) {
        minX = aMinX;
    }
    if (aMinY < minY) {
        minY = aMinY;
    }
    if (aMaxX > maxX) {
        maxX = aMaxX;
    }
    if (aMaxY > maxY) {
        maxY = aMaxY;
    }
}

float RegionsCluster::distance(ConnectedRegion &region) {
    float distance = 0;
    if (region.minX > maxX) {
        distance += region.minX - maxX;
    }
    if (region.maxX < minX) {
        distance += minX - region.maxX;
    }
    if (region.minY > maxY) {
        distance += region.minY - maxY;
    }
    if (region.maxY < minY) {
        distance += minY - region.maxY;
    }
    return distance;
}

float RegionsCluster::alignment(ConnectedRegion &region) {
    float height = min(region.height(), (float)maxY-minY);
    float topDiff = (region.maxY - maxY) / height;
    float bottomDiff = (region.minY - minY) / height;
    return sqrt(topDiff*topDiff + bottomDiff*bottomDiff);
}

float RegionsCluster::averageArea() {
    float areaAccumulator = 0;
    list<ConnectedRegion>::iterator regionsIter;
    for (regionsIter = regions.begin(); regionsIter != regions.end(); regionsIter++) {
        areaAccumulator += regionsIter->area();
    }
    return areaAccumulator / regions.size();
}

float RegionsCluster::width() {
    return maxX - minY;
}

float RegionsCluster::height() {
    return maxY - minY;
}

float RegionsCluster::area() {
    return width() * height();
}

bool RegionsCluster::operator<(RegionsCluster other) {
    return (area()) < (other.area());
}

RegionBounds RegionsCluster::bounds() {
  RegionBounds bounds;
  bounds.top = minY;
  bounds.left = minX;
  bounds.bottom = maxY;
  bounds.right = maxX;
  return bounds;
}

void RegionsCluster::addPadding(int padding, int width, int height) {
    minX = max(0, minX - padding);
    minY = max(0, minY - padding);
    maxX = min(width-1, maxX + padding);
    maxY = min(height-1, maxY + padding);
}

bool RegionsCluster::isWholeImage(int width, int height) {
    return ((minX <= 0) &&
            (minY <= 0) &&
            (maxX >= width-1) &&
            (maxY >= height-1));
}

bool RegionsCluster::insideBounds(RegionBounds &bounds) {
    return ((minX >= bounds.left) &&
            (minY >= bounds.top) &&
            (maxX <= bounds.right) &&
            (maxY <= bounds.bottom));
}

int RegionsCluster::blobsCount() {
    return regions.size();
}

