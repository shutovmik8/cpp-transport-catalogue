#include "transport_catalogue.h"

#include <algorithm>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>
#include <cassert>


namespace transport_catalogue {

std::optional<int> TransportCatalogue::GetDistance(const std::string_view stop1_name, const std::string_view stop2_name) const {
    auto stop1_it = stops_names.find(stop1_name);
    assert(stop1_it != stops_names.end());
    auto stop2_it = stops_names.find(stop2_name);
    assert(stop2_it != stops_names.end());
    auto it = distances_.find({stop1_it->second, stop2_it->second});
    if (it == distances_.end()) {
        it = distances_.find({stop2_it->second, stop1_it->second});
    }
    if (it == distances_.end()) {
        return std::nullopt;
    }
    return it->second;
}
    
void TransportCatalogue::AddDistance(const std::string_view stop1_name, const std::string_view stop2_name, int distance) {
    distances_[{stops_names[stop1_name], stops_names[stop2_name]}] = distance;
}    
    
void TransportCatalogue::AddBus(std::string bus_name, const std::vector<std::string_view>& stops) {
    std::vector<Stop*> bus_stops;
    for (auto& stop: stops) { 
        bus_stops.push_back(stops_names[stop]);
    }
    buses_.push_back({std::move(bus_name), std::move(bus_stops)});
    buses_names[buses_.back().name] = &buses_.back();
}

void TransportCatalogue::AddStop(std::string stop_name, const Coordinates& stop_coord) {
    stops_.push_back({std::move(stop_name), stop_coord}); 
    stops_names[stops_.back().name] = &stops_.back();
}

void TransportCatalogue::AddSpeedAndWait(double speed, double wait) {
    bus_speed = speed;
    bus_wait_time = wait;
}

double TransportCatalogue::GetSpeed() const {
    return bus_speed;
}

double TransportCatalogue::GetWaitTime() const {
    return bus_wait_time;
}

const std::deque<Stop>& TransportCatalogue::GetStops() const {
    return stops_;
}

const std::deque<Bus>& TransportCatalogue::GetBuses() const {
    return buses_;
}

bool TransportCatalogue::GetIsRoundtrip(const std::string_view& name) const{
    return is_roundtrip.at(name);
}

void TransportCatalogue::AddRoundtripInfo(const std::string_view& name, bool is_roundtrip_) {
    is_roundtrip[name] = is_roundtrip_;
}

const Bus* TransportCatalogue::FindBus(const std::string_view name) const {
    auto it = buses_names.find(name);
    if (it != buses_names.end()) {
        return it->second;
    }
    return nullptr;
}

const Stop* TransportCatalogue::FindStop(const std::string_view name) const {
    auto it = stops_names.find(name);
    if (it != stops_names.end()) {
        return it->second;
    }
    return nullptr;
}

std::optional<std::set<std::string_view>> TransportCatalogue::GetBusesForStop(const std::string_view name) const {
    if (!stops_names.contains(name)) {
        return std::nullopt;
    }
    std::set<std::string_view> buses_for_stop;
    for (auto& bus : buses_) {
        auto it = std::find_if(bus.stops.begin(), bus.stops.end(), [name](Stop* stop) { return stop->name == name; });
        if (it != bus.stops.end()) {
            buses_for_stop.insert(bus.name);
        }
    }
    return buses_for_stop;
}

std::optional<BusInfo> TransportCatalogue::GetBusInfo(const std::string_view name) const {
    auto it = buses_names.find(name);
    if (it == buses_names.end()) {
        return std::nullopt;
    }
    std::unordered_set<Stop*> unique_stops;
    size_t amount = 0;
    double length = 0;
    double real_length = 0;
    Stop* last_stop = nullptr;
    for (auto& stop_ptr : it->second->stops) {
        if (amount) {
            length += ComputeDistance(last_stop->coordinates, stop_ptr->coordinates);
            real_length += TransportCatalogue::GetDistance(last_stop->name, stop_ptr->name).value();
        }
        last_stop = stop_ptr;
        if (unique_stops.contains(stop_ptr)) {
            continue;
        }
        unique_stops.insert(stop_ptr);
        ++amount;
    }
    return BusInfo{it->second->stops.size(), amount, real_length, real_length / length};
}

} //transport_catalogue