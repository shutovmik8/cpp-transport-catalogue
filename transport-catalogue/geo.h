#pragma once

#include <cmath>

namespace transport_catalogue {

struct Coordinates {
    double lat;
    double lng;
    bool operator==(const Coordinates& other) const;
    bool operator<(const Coordinates& other) const;
    bool operator!=(const Coordinates& other) const;
};

namespace detail {

double ComputeDistance(Coordinates from, Coordinates to);

}

}
