#include <iomanip>
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_set>

#include "stat_reader.h"

namespace trancport_catalogue {

namespace detail {  

void PrintBus(const TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& output) {
    Bus* bus = tansport_catalogue.FindBus(request);
    if (!bus) {
        output << "Bus " << std::string(request) << ": not found" << std::endl;
        return;
    }
    std::unordered_set<Stop*> unique_stops;
    size_t amount = 0;
    double length = 0;
    Stop* last_stop = nullptr;
    for (auto& stop_ptr : bus->stops) {
        if (amount) {
            length += ComputeDistance(last_stop->coordinates, stop_ptr->coordinates);
        }
        if (unique_stops.contains(stop_ptr)) {
            last_stop = stop_ptr;
            continue;
        }
        last_stop = stop_ptr;
        unique_stops.insert(stop_ptr);
        ++amount;
    }
    output << "Bus " << std::string(request) << ": " << bus->stops.size() << " stops on route, " << amount << " unique stops, " << std::setprecision(6) << length << " route length" << std::endl;
}

void PrintStop(const TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& output) {
    auto buses_for_stop = tansport_catalogue.FindStop(request);
    if (!buses_for_stop.has_value()) {
        output << "Stop " << request << ": not found" << std::endl;
        return;
    }
    if (buses_for_stop->size() == 0) {
        output << "Stop " << request << ": no buses" << std::endl;
        return;
    }
    output << "Stop " << request << ": buses";
    for (auto& bus_name : *buses_for_stop) {
        output << " " << std::string(bus_name);
    }
    output << std::endl;
}

}

void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output) {
    auto pos = request.find(' ');
    if (request.substr(0, pos) == "Bus") {
        detail::PrintBus(tansport_catalogue, request.substr(pos + 1), output);
    }
    else if (request.substr(0, pos) == "Stop") {
        detail::PrintStop(tansport_catalogue, request.substr(pos + 1), output);
    }
}

}