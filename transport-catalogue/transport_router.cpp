#include "transport_router.h"

namespace graph {

DirectedWeightedGraph<double> RoutesManager::MakeRoutesGraph(const transport_catalogue::TransportCatalogue& catalogue) {
    const int meters_in_kilometer = 1000;
    const int second_in_minute = 60;
    const auto& stops = catalogue.GetStops();
    graph::DirectedWeightedGraph<double> graph(2 * stops.size());
    size_t stop_num = 0;
    for (const auto& stop : stops) {
        graph.stops_id[stop.name] = stop_num + 1;
        graph.stops_name[stop_num + 1] = stop.name;
        stop_num += 2;
    }
    const auto& buses = catalogue.GetBuses();
    std::unordered_set<std::string_view> self_edge_stop;
    for (const auto& bus : buses) {
        bool is_roundtrip = catalogue.GetIsRoundtrip(bus.name);
        auto stops_names_vector = (is_roundtrip ? bus.stops : std::vector<transport_catalogue::Stop*>(bus.stops.begin(), bus.stops.begin() + bus.stops.size() / 2 + 1));
        for (auto it_1 = stops_names_vector.begin(); it_1 != stops_names_vector.end(); ++it_1) {
            if (!self_edge_stop.contains((*it_1)->name)) {
                size_t edge_id = graph.AddEdge({static_cast<size_t>(graph.stops_id[(*it_1)->name]) - 1, static_cast<size_t>(graph.stops_id[(*it_1)->name]), catalogue.GetWaitTime()});
                graph.edges_info[edge_id] = {"", 0}; 
                self_edge_stop.insert((*it_1)->name);            
            }
            double sum_time = 0;
            for (auto it_2 = std::next(it_1); it_2 != stops_names_vector.end(); ++it_2) {
                if ((is_roundtrip) and (it_1 == stops_names_vector.begin()) and (it_2 == std::prev(stops_names_vector.end()))) {
                    break;
                }
                sum_time += catalogue.GetDistance((*std::prev(it_2))->name, (*it_2)->name).value() / (catalogue.GetSpeed() * meters_in_kilometer / second_in_minute);
                size_t edge_id = graph.AddEdge({static_cast<size_t>(graph.stops_id[(*it_1)->name]), static_cast<size_t>(graph.stops_id[(*it_2)->name]) - 1, sum_time});
                auto dist = std::distance(it_1, it_2);
                graph.edges_info[edge_id] = {bus.name, static_cast<int>(dist > 0 ? dist : dist * -1)};
            }
            if ((!is_roundtrip) and (it_1 != stops_names_vector.begin())) {
                double sum_time_back = 0;
                for (auto it_3 = it_1; it_3 != stops_names_vector.begin(); --it_3) {
                    sum_time_back += catalogue.GetDistance((*it_3)->name, (*std::prev(it_3))->name).value() / (catalogue.GetSpeed() * meters_in_kilometer / second_in_minute);
                    size_t edge_id = graph.AddEdge({static_cast<size_t>(graph.stops_id[(*it_1)->name]), static_cast<size_t>(graph.stops_id[(*std::prev(it_3))->name]) - 1, sum_time_back});
                    auto dist = std::distance(it_1, std::prev(it_3));
                    graph.edges_info[edge_id] = {bus.name, static_cast<int>(dist > 0 ? dist : dist * -1)};
                }        
            }
        }
    }
    return graph;
}

std::optional<RouteInfo> RoutesManager::GetRoute(std::string_view from, std::string_view to) const {
    auto route_info = router.BuildRoute(static_cast<size_t>(graph.stops_id.at(from)) - 1, static_cast<size_t>(graph.stops_id.at(to)) - 1);
    if (!route_info.has_value()) {
        return std::nullopt;
    }
    std::vector<std::variant<BusRiding, Waiting>> route_units;
    for (auto item : route_info.value().edges) {
        if (graph.GetEdge(item).from % 2 == 0) {
            route_units.push_back(Waiting{graph.stops_name.at(graph.GetEdge(item).to), graph.GetEdge(item).weight});
        }
        else {
            route_units.push_back(BusRiding{graph.edges_info.at(item).bus_name, graph.edges_info.at(item).stops_count, graph.GetEdge(item).weight});         
        } 
    }
    return RouteInfo{route_info.value().weight, route_units};
}

}