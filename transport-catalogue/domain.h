#pragma once

#include "geo.h"

#include <vector>
#include <string>
#include <string_view>
#include <utility>

namespace transport_catalogue {

struct Stop {
    std::string name;
    Coordinates coordinates;
};

struct Bus {
    std::string name;
    std::vector<Stop*> stops;
};
    
struct RouteInfo {
    std::vector<Stop> stops;
    bool is_roundtrip;
};
    
}