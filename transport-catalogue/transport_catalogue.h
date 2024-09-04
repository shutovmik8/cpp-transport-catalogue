#pragma once

#include <deque>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "geo.h"

namespace trancport_catalogue {

struct Stop {
    std::string name;
    Coordinates coordinates;
};

struct Bus {
    std::string name;
    std::vector<Stop*> stops;
};

struct BusInfo {
    size_t stops_on_route{};
    size_t unique_stops{};
    double length{};
};

class TransportCatalogue {
    std::unordered_map<std::string_view, Stop*> stops_names;
    std::unordered_map<std::string_view, Bus*> buses_names;
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    
public: 
    
    void AddBus(std::string bus_name, const std::vector<std::string_view>& stops);
    void AddStop(std::string stop_name, const Coordinates& stop_coord);
    Bus* FindBus(const std::string_view& name) const;
    std::optional<std::set<std::string_view>> BusesForStop(const std::string_view& name) const;
    BusInfo GetBusInfo(const std::string_view& name) const;
};

}