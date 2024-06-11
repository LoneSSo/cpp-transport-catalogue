#pragma once

#include <cmath>

namespace geo{
struct Coordinates {
    double lat;
    double lng;
    bool operator==(const Coordinates& other) const {
        return lat == other.lat && lng == other.lng;
    }
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
};

struct Distance {
    double geo;
    double road;
};

double ComputeDistance(Coordinates from, Coordinates to);
}