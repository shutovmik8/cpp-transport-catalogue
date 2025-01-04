#pragma once
#include <vector>
#include <string_view>
#include <string>

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace transport_catalogue {
  
std::map<std::string_view, RouteInfo> GetAllBuses(const TransportCatalogue& tansport_catalogue, const json::Node& catalogue_data); //Используется при отрисовки карты  
json::Document ParseAndMakeAnswers(const TransportCatalogue& tansport_catalogue, const json::Node& catalogue_data);
void LoadCatalogueFromJson(TransportCatalogue& catalogue, const json::Node& root);
    
}