#include <algorithm>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "transport_catalogue.h"

namespace trancport_catalogue {

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

Bus* TransportCatalogue::FindBus(const std::string_view& name) const {
    auto it = buses_names.find(name);
    if (it != buses_names.end()) {
        return it->second;
    }
    return nullptr;
}

std::optional<std::set<std::string_view>> TransportCatalogue::BusesForStop(const std::string_view& name) const {
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

BusInfo TransportCatalogue::GetBusInfo(const std::string_view& name) const {
    auto it = buses_names.find(name);
    if (it == buses_names.end()) {
        return BusInfo{};
    }
    std::unordered_set<Stop*> unique_stops;
    size_t amount = 0;
    double length = 0;
    Stop* last_stop = nullptr;
    for (auto& stop_ptr : it->second->stops) {
        if (amount) {
            length += detail::ComputeDistance(last_stop->coordinates, stop_ptr->coordinates);
        }
        if (unique_stops.contains(stop_ptr)) {
            last_stop = stop_ptr;
            continue;
        }
        last_stop = stop_ptr;
        unique_stops.insert(stop_ptr);
        ++amount;
    }
    return {it->second->stops.size(), amount, length};
}

}