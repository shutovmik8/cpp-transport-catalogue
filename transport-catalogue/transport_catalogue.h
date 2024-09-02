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

class TransportCatalogue {
    std::unordered_map<std::string_view, Stop*> stops_names;
    std::unordered_map<std::string_view, Bus*> buses_names;
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    
public: 
    
    void AddBus(std::string, const std::vector<std::string_view>&);
    void AddStop(std::string, const Coordinates&);
    Bus* FindBus(const std::string_view&) const;
    std::optional<std::set<std::string_view>> FindStop(const std::string_view&) const;
    std::string GetBusInfo(const std::string_view&) const;
};

}