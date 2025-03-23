#include "json_builder.h"
#include "router.h"
#include "transport_router.h"

#include <vector>
#include <string_view>
#include <string>

namespace transport_catalogue {

void GetRoute(const graph::Router<double>& router, graph::DirectedWeightedGraph<double>& graph, const json::Node& request, json::Builder& builder) {
    auto route_info = router.BuildRoute(static_cast<size_t>(graph.stops_id[request.AsMap().at("from").AsString()]) - 1, static_cast<size_t>(graph.stops_id[request.AsMap().at("to").AsString()]) - 1);
    builder.StartDict();
    if (!route_info.has_value()) {
        builder.Key("request_id").Value(request.AsMap().at("id").AsInt()).Key("error_message").Value("not found").EndDict();
        return;
    }
    builder.Key("request_id").Value(request.AsMap().at("id").AsInt()).Key("total_time").Value(route_info.value().weight).Key("items").StartArray();
    for (auto item : route_info.value().edges) {
        if (graph.GetEdge(item).from % 2 == 0) {
            builder.StartDict().Key("type").Value("Wait").Key("stop_name").Value(std::string(graph.stops_name[graph.GetEdge(item).to])).Key("time").Value(graph.GetEdge(item).weight).EndDict();             
            continue;
        }
        else {
            builder.StartDict().Key("type").Value("Bus").Key("bus").Value(std::string(graph.edges_info[item].bus_name))
            .Key("span_count").Value(graph.edges_info[item].stops_count).Key("time").Value(graph.GetEdge(item).weight).EndDict();
            continue;              
        } 
    }
    builder.EndArray().EndDict();
}

}