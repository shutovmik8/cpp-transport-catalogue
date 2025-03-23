#include "json_reader.h"
#include "json_builder.h"
#include "router.h"
#include "transport_router.h"

#include <iterator>
#include <vector>
#include <set>
#include <string_view>
#include <string>

namespace transport_catalogue {

using namespace std::literals;

void GetBusStat(const TransportCatalogue& tansport_catalogue, const json::Node& request, json::Builder& builder) {
    auto bus_info = tansport_catalogue.GetBusInfo(request.AsMap().at("name").AsString());
    builder.StartDict().Key("request_id").Value(request.AsMap().at("id").AsInt());
    if (!bus_info.has_value()) { 
        builder.Key("error_message").Value("not found").EndDict();
        return;
    }
    builder.Key("curvature").Value(bus_info->curvature)
        .Key("route_length").Value(bus_info->length)
        .Key("stop_count").Value(static_cast<int>(bus_info->stops_on_route))
        .Key("unique_stop_count").Value(static_cast<int>(bus_info->unique_stops)).EndDict();
}

void GetStopStat(const TransportCatalogue& tansport_catalogue, const json::Node& request, json::Builder& builder) {
    auto buses_for_stop = tansport_catalogue.GetBusesForStop(request.AsMap().at("name").AsString());
    builder.StartDict().Key("request_id").Value(request.AsMap().at("id").AsInt());
    if (!buses_for_stop.has_value()) {
        builder.Key("error_message").Value("not found").EndDict();
        return;
    }
    json::Array buses;
    builder.Key("buses").StartArray();
    for (auto& bus_name : *buses_for_stop) {
        builder.Value(std::string{bus_name});
    }
    builder.EndArray().EndDict();
}

std::vector<std::string_view> MakeStopsNamesVector(const std::vector<json::Node>& stops) {
    std::vector<std::string_view> results;
    for (const auto& stop : stops) {
        results.push_back(stop.AsString());
    }
    return results;
}


//В случае некольцевого маршрута добавляет остановки от конца к началу. Было A->B->C, стало A->B->C->B->A
std::vector<std::string_view> ParseRoute(const std::vector<std::string_view>& stops) {
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());
    return results;
}

svg::Color GetColorFromJson(const json::Node& color) {
    if (color.IsString()) {
        return color.AsString();
    }
    else if ((color.IsArray()) and (color.AsArray().size() == 3)) {
        return svg::Rgb{static_cast<uint8_t>(color.AsArray().at(0).AsInt()), static_cast<uint8_t>(color.AsArray().at(1).AsInt()), static_cast<uint8_t>(color.AsArray().at(2).AsInt())};
    }
    else if ((color.IsArray()) and (color.AsArray().size() == 4)) {
        return svg::Rgba{static_cast<uint8_t>(color.AsArray().at(0).AsInt()), static_cast<uint8_t>(color.AsArray().at(1).AsInt()), static_cast<uint8_t>(color.AsArray().at(2).AsInt()), color.AsArray().at(3).AsDouble()};
    }
    return svg::Color();
}

RenderSettings ParseRenderSettings(const json::Node& catalogue_data) {
    const auto& stat_requests = catalogue_data.AsMap().at("render_settings").AsMap();
    std::vector<svg::Color> colors;
    for (auto& color : stat_requests.at("color_palette").AsArray()) {
        colors.push_back(GetColorFromJson(color));
    }
    return RenderSettings{
        stat_requests.at("width").AsDouble(),
        stat_requests.at("height").AsDouble(),
        stat_requests.at("padding").AsDouble(),
        stat_requests.at("stop_radius").AsDouble(),
        stat_requests.at("line_width").AsDouble(),
        stat_requests.at("bus_label_font_size").AsInt(),
        {stat_requests.at("bus_label_offset").AsArray().at(0).AsDouble(), stat_requests.at("bus_label_offset").AsArray().at(1).AsDouble()},
        stat_requests.at("stop_label_font_size").AsInt(),
        {stat_requests.at("stop_label_offset").AsArray().at(0).AsDouble(), stat_requests.at("stop_label_offset").AsArray().at(1).AsDouble()},
        GetColorFromJson(stat_requests.at("underlayer_color")),
        stat_requests.at("underlayer_width").AsDouble(),
        colors
    };
}
 
std::map<std::string_view, RouteInfo> GetAllBuses(const TransportCatalogue& tansport_catalogue, const json::Node& catalogue_data) {
    const auto& base_requests = catalogue_data.AsMap().at("base_requests").AsArray();
    std::map<std::string_view, RouteInfo> answer;
    for (const auto& request : base_requests) {
        if (request.AsMap().at("type").AsString() == "Bus") {
            const Bus* bus = tansport_catalogue.FindBus(request.AsMap().at("name").AsString());
            if ((bus->stops.empty()) || (bus->stops.size() < 3)) {
                continue;
            }
            std::vector<Stop> stops;
            for (const auto* stop : bus->stops) {
                stops.push_back(*stop);
            }
            answer.insert({bus->name, {std::move(stops), request.AsMap().at("is_roundtrip").AsBool()}});
        }  
        
    }
    return answer;
} 

size_t AddStopsToCatalogue(TransportCatalogue& catalogue, const json::Node& root) {
    size_t stops_count = 0;
    const auto& base_requests = root.AsMap().at("base_requests").AsArray();
    for (const auto& base_request_data : base_requests) {
        const auto& base_request_data_map = base_request_data.AsMap();
        if (base_request_data_map.at("type").AsString() == "Stop") {
            ++stops_count;
            catalogue.AddStop(base_request_data_map.at("name").AsString(), {base_request_data_map.at("latitude").AsDouble(), 
                base_request_data_map.at("longitude").AsDouble()});
        }
    }
    return stops_count;
}

void FillCatalogueAndGraph(graph::DirectedWeightedGraph<double>& graph, TransportCatalogue& catalogue, const json::Node& root) {
    int stop_num = 0;
    const auto& base_requests = root.AsMap().at("base_requests").AsArray();
    for (const auto& base_request_data : base_requests) {
        const auto& base_request_data_map = base_request_data.AsMap();
        if (base_request_data_map.at("type").AsString() == "Stop") {
            graph.stops_id[base_request_data_map.at("name").AsString()] = stop_num + 1;
            graph.stops_name[stop_num + 1] = base_request_data_map.at("name").AsString();
            stop_num += 2;
            for (auto& road_distance : base_request_data_map.at("road_distances").AsMap()) {
                catalogue.AddDistance(base_request_data_map.at("name").AsString(), road_distance.first, road_distance.second.AsInt());
            }
        }
    }
}

void AddBusesToCatalogueAndGraph(graph::DirectedWeightedGraph<double>& graph, TransportCatalogue& catalogue, const json::Node& root) {
    const int meters_in_kilometer = 1000;
    const int second_in_minute = 60;
    const auto& base_requests = root.AsMap().at("base_requests").AsArray();
    double speed = root.AsMap().at("routing_settings").AsMap().at("bus_velocity").AsDouble();
    double waiting = root.AsMap().at("routing_settings").AsMap().at("bus_wait_time").AsInt();
    std::unordered_set<std::string_view> self_edge_stop;
    for (const auto& base_request_data : base_requests) {
        const auto& base_request_data_map = base_request_data.AsMap();
        if (base_request_data_map.at("type").AsString() == "Bus") {
            auto stops_names_vector = MakeStopsNamesVector(base_request_data_map.at("stops").AsArray());
            catalogue.AddBus(base_request_data_map.at("name").AsString(), (base_request_data_map.at("is_roundtrip").AsBool() ? stops_names_vector : ParseRoute(stops_names_vector)));
            for (auto it_1 = stops_names_vector.begin(); it_1 != stops_names_vector.end(); ++it_1) {
                if (!self_edge_stop.contains(*it_1)) {
                    size_t edge_id = graph.AddEdge({static_cast<size_t>(graph.stops_id[*it_1]) - 1, static_cast<size_t>(graph.stops_id[*it_1]), waiting});
                    graph.edges_info[edge_id] = {"", 0}; 
                    self_edge_stop.insert(*it_1);            
                }
                double sum_time = 0;
                for (auto it_2 = std::next(it_1); it_2 != stops_names_vector.end(); ++it_2) {
                    if ((base_request_data_map.at("is_roundtrip").AsBool()) and (it_1 == stops_names_vector.begin()) and (it_2 == std::prev(stops_names_vector.end()))) {
                        break;
                    }
                    sum_time += catalogue.GetDistance(*std::prev(it_2), *it_2).value() / (speed * meters_in_kilometer / second_in_minute);
                    size_t edge_id = graph.AddEdge({static_cast<size_t>(graph.stops_id[*it_1]), static_cast<size_t>(graph.stops_id[*it_2]) - 1, sum_time});
                    auto dist = std::distance(it_1, it_2);
                    graph.edges_info[edge_id] = {base_request_data_map.at("name").AsString(), static_cast<int>(dist > 0 ? dist : dist * -1)};
                }
                if ((!base_request_data_map.at("is_roundtrip").AsBool()) and (it_1 != stops_names_vector.begin())) {
                    double sum_time_back = 0;
                    for (auto it_3 = it_1; it_3 != stops_names_vector.begin(); --it_3) {
                        sum_time_back += catalogue.GetDistance(*it_3, *std::prev(it_3)).value() / (speed * meters_in_kilometer / second_in_minute);
                        size_t edge_id = graph.AddEdge({static_cast<size_t>(graph.stops_id[*it_1]), static_cast<size_t>(graph.stops_id[*std::prev(it_3)]) - 1, sum_time_back});
                        auto dist = std::distance(it_1, std::prev(it_3));
                        graph.edges_info[edge_id] = {base_request_data_map.at("name").AsString(), static_cast<int>(dist > 0 ? dist : dist * -1)};
                    }        
                }
            }
        }
    }
}

graph::DirectedWeightedGraph<double> LoadCatalogueFromJson(TransportCatalogue& catalogue, const json::Node& root) {
    size_t stops_count = AddStopsToCatalogue(catalogue, root);
    graph::DirectedWeightedGraph<double> graph(2 * stops_count);
    FillCatalogueAndGraph(graph, catalogue, root);
    AddBusesToCatalogueAndGraph(graph, catalogue, root);
    return graph;
}

json::Document ParseAndMakeAnswers(const TransportCatalogue& tansport_catalogue, const json::Node& catalogue_data, graph::DirectedWeightedGraph<double>& graph) {
    const auto& stat_requests = catalogue_data.AsMap().at("stat_requests").AsArray();
    json::Builder builder{};
    builder.StartArray();
    graph::Router<double> router(graph);
    for (const auto& request : stat_requests) {
        if (request.AsMap().at("type").AsString() == "Bus") {
            GetBusStat(tansport_catalogue, request, builder);
        }
        else if (request.AsMap().at("type").AsString() == "Stop") {
            GetStopStat(tansport_catalogue, request, builder);
        }
        else if (request.AsMap().at("type").AsString() == "Map") {
            builder.StartDict().Key("request_id").Value(request.AsMap().at("id").AsInt()).Key("map").Value(GetMapJson(ParseRenderSettings(catalogue_data), GetAllBuses(tansport_catalogue, catalogue_data))).EndDict();
        }
        else if (request.AsMap().at("type").AsString() == "Route") {
            GetRoute(router, graph, request, builder);
        }
    }
    return json::Document{builder.EndArray().Build()};
}
    
} //trancport_catalogue