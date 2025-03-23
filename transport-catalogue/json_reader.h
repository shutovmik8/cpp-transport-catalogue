#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "domain.h"
#include "graph.h"

#include <vector>
#include <string_view>
#include <string>

namespace transport_catalogue {
  
std::map<std::string_view, RouteInfo> GetAllBuses(const TransportCatalogue& tansport_catalogue, const json::Node& catalogue_data); //Используется при отрисовки карты  
json::Document ParseAndMakeAnswers(const TransportCatalogue& tansport_catalogue, const json::Node& catalogue_data, graph::DirectedWeightedGraph<double>& graph);
graph::DirectedWeightedGraph<double> LoadCatalogueFromJson(TransportCatalogue& catalogue, const json::Node& root);
    
}