#pragma once

#include "geo.h"
#include "domain.h"

#include <deque>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace transport_catalogue {

struct BusInfo {
    size_t stops_on_route{};
    size_t unique_stops{};
    double length{};
    double curvature{};
};

class PairStopsHasher {
public:
    size_t operator()(const std::pair<Stop*, Stop*>& item) const {
        return std::hash<const void*>{}(item.first) + 37 * std::hash<const void*>{}(item.second);
    }
};
    
class TransportCatalogue {
    std::unordered_map<std::pair<Stop*, Stop*>, int, PairStopsHasher> distances_;
    std::unordered_map<std::string_view, Stop*> stops_names;
    std::unordered_map<std::string_view, Bus*> buses_names;
    std::unordered_map<std::string_view, bool> is_roundtrip;
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    double bus_speed;
    double bus_wait_time;
    
public: 
    void AddDistance(const std::string_view stop1_name, const std::string_view stop2_name, int distance);
    std::optional<int> GetDistance(const std::string_view stop1_name, const std::string_view stop2_name) const;
    void AddBus(std::string bus_name, const std::vector<std::string_view>& stops);
    void AddStop(std::string stop_name, const Coordinates& stop_coord);
    void AddSpeedAndWait(double speed, double wait);
    const Bus* FindBus(const std::string_view name) const;
    const Stop* FindStop(const std::string_view name) const;
    double GetSpeed() const;
    double GetWaitTime() const;
    void AddRoundtripInfo(const std::string_view& name, bool is_roundtrip_);
    bool GetIsRoundtrip(const std::string_view& name) const;
    const std::deque<Stop>& GetStops() const;
    const std::deque<Bus>& GetBuses() const;
    std::optional<std::set<std::string_view>> GetBusesForStop(const std::string_view name) const;
    std::optional<BusInfo> GetBusInfo(const std::string_view name) const;
};

} //transport_catalogue