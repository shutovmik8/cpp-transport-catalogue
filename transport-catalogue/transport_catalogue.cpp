#include <algorithm>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "transport_catalogue.h"

namespace trancport_catalogue {

void TransportCatalogue::AddDistance(std::string stop_name, std::unordered_map<std::string, int> distances) {
    for (auto& item : distances) {
        distances_[{stops_names[stop_name], stops_names[item.first]}] = item.second;
    }
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
            length += detail::ComputeDistance(last_stop->coordinates, stop_ptr->coordinates);
            auto it = distances_.find({last_stop, stop_ptr});
            if (it == distances_.end()) {
                real_length += distances_.at({stop_ptr, last_stop});
            }
            else {
                real_length += it->second;
            }
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

}