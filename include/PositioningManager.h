#ifndef POSITIONING_MANAGER_H
#define POSITIONING_MANAGER_H

#include <map>
#include "DataUtils.h"

struct Point {
    float x = 0.0f, y = 0.0f, z = 0.0f;
};

class PositioningManager {
public:
    PositioningManager(int minAnchors = 3);
    void setAnchorPosition(uint16_t anchor_saddr, float x, float y, float z);
    void addAnchorReport(const DecodedAnchorReport_t& report);
    Point getLastTagPosition() const;
    const std::map<uint16_t, DecodedAnchorReport_t>& getAnchorDataMap() const;

private:
    void calculateTagPosition(uint16_t sequence_id);

    int _minAnchors;
    Point _lastTagPosition;
    std::map<uint16_t, Point> _anchorPositions;
    std::map<uint16_t, DecodedAnchorReport_t> _latestAnchorData;
    std::map<uint16_t, std::map<uint16_t, DecodedAnchorReport_t>> _sequenceData;
};

#endif // POSITIONING_MANAGER_H
