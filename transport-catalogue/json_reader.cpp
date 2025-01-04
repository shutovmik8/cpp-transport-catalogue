#include <iterator>
#include <vector>
#include <string_view>
#include <string>

#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
namespace transport_catalogue {

using namespace std::literals;

json::Node GetBusStat(const TransportCatalogue& tansport_catalogue, const json::Node& request) {
    auto bus_info = tansport_catalogue.GetBusInfo(request.AsMap().at("name").AsString());
    json::Dict answer;
    answer.insert({"request_id", json::Node{request.AsMap().at("id").AsInt()}});
    if (!bus_info.has_value()) {
        answer.insert({"error_message", json::Node{"not found"s}});
        return json::Node{answer};
    }
    answer.insert({"curvature", json::Node{bus_info->curvature}});
    answer.insert({"route_length", json::Node{bus_info->length}});
    answer.insert({"stop_count", json::Node{static_cast<int>(bus_info->stops_on_route)}});
    answer.insert({"unique_stop_count", json::Node{static_cast<int>(bus_info->unique_stops)}});
    return json::Node{answer};
}
    
json::Node GetStopStat(const TransportCatalogue& tansport_catalogue, const json::Node& request) {
    auto buses_for_stop = tansport_catalogue.GetBusesForStop(request.AsMap().at("name").AsString());
    json::Dict answer;
    answer.insert({"request_id", json::Node{request.AsMap().at("id").AsInt()}});
    if (!buses_for_stop.has_value()) {
        answer.insert({"error_message", json::Node{"not found"s}});
        return json::Node{answer};
    }
    json::Array buses;
    for (auto& bus_name : *buses_for_stop) {
        buses.push_back(json::Node{std::string{bus_name}});
    }
    answer.insert({"buses", json::Node{buses}});
    return json::Node{answer};

}

std::vector<std::string_view> MakeStopsNamesVector(const std::vector<json::Node>& stops) {
    std::vector<std::string_view> results;
    for (const auto& stop : stops) {
        results.push_back(stop.AsString());
    }
    return results;
}


//В случае некольцевого маршрута добавляет остановки от конца к началу. Было A->B->C, стало A->B->C->B->C
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
 
//} //detail
 
std::map<std::string_view, RouteInfo> GetAllBuses(const TransportCatalogue& tansport_catalogue, const json::Node& catalogue_data) {
    const auto& base_requests = catalogue_data.AsMap().at("base_requests").AsArray();
    std::map<std::string_view, RouteInfo> answer;
    for (const auto& request : base_requests) {
        if (request.AsMap().at("type").AsString() == "Bus") {
            const Bus* bus = tansport_catalogue.FindBus(request.AsMap().at("name").AsString());
            if ((bus->stops.empty()) or (bus->stops.size() < 3)) {
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

void LoadCatalogueFromJson(TransportCatalogue& catalogue, const json::Node& root) {
    const auto& base_requests = root.AsMap().at("base_requests").AsArray();
    for (const auto& base_request_data : base_requests) {
        const auto& base_request_data_map = base_request_data.AsMap();
        if (base_request_data_map.at("type").AsString() == "Stop") {
            catalogue.AddStop(base_request_data_map.at("name").AsString(), {base_request_data_map.at("latitude").AsDouble(), 
                base_request_data_map.at("longitude").AsDouble()});
        }
    }
    for (const auto& base_request_data : base_requests) {
        const auto& base_request_data_map = base_request_data.AsMap();
        if (base_request_data_map.at("type").AsString() == "Stop") {
            for (auto& road_distance : base_request_data_map.at("road_distances").AsMap()) {
                catalogue.AddDistance(base_request_data_map.at("name").AsString(), road_distance.first, road_distance.second.AsInt());
            }
        }
    }
    for (const auto& base_request_data : base_requests) {
        const auto& base_request_data_map = base_request_data.AsMap();
        if (base_request_data_map.at("type").AsString() == "Bus") {
            const auto& stops_names_vector = MakeStopsNamesVector(base_request_data_map.at("stops").AsArray());
            catalogue.AddBus(base_request_data_map.at("name").AsString(), base_request_data_map.at("is_roundtrip").AsBool() ? 
                stops_names_vector : ParseRoute(stops_names_vector));
        }
    }
}

json::Document ParseAndMakeAnswers(const TransportCatalogue& tansport_catalogue, const json::Node& catalogue_data) {
    const auto& stat_requests = catalogue_data.AsMap().at("stat_requests").AsArray();
    std::vector<json::Node> answers;
    for (const auto& request : stat_requests) {
        if (request.AsMap().at("type").AsString() == "Bus") {
            answers.push_back(GetBusStat(tansport_catalogue, request));
        }
        else if (request.AsMap().at("type").AsString() == "Stop") {
            answers.push_back(GetStopStat(tansport_catalogue, request));
        }
        else if (request.AsMap().at("type").AsString() == "Map") {
            json::Dict map_answer;
            map_answer.insert({"request_id", request.AsMap().at("id").AsInt()});
            map_answer.insert({"map", GetMapJson(ParseRenderSettings(catalogue_data), GetAllBuses(tansport_catalogue, catalogue_data))});
            answers.push_back(json::Node(map_answer));
        }
    }
    return json::Document{json::Node{answers}};
}
    
} //trancport_catalogue