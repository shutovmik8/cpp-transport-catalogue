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
    auto bus_info = tansport_catalogue.GetBusInfo(request);
    if (!bus_info.has_value()) {
        output << "Bus " << std::string(request) << ": not found" << std::endl;
        return;
    }
    output << "Bus " << std::string(request) << ": " << bus_info->stops_on_route << " stops on route, " << bus_info->unique_stops << " unique stops, " << std::setprecision(6) << bus_info->length << " route length" << std::endl;
}

void PrintStop(const TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& output) {
    auto buses_for_stop = tansport_catalogue.GetBusesForStop(request);
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