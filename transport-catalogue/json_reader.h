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

namespace detail {
    json::Node GetBusStat(const TransportCatalogue& tansport_catalogue, const json::Node& request); //Возвращает ответ на запрос Bus в форме json
    json::Node GetStopStat(const TransportCatalogue& tansport_catalogue, const json::Node& request); //Возвращает ответ на запрос Stop в форме json
    std::vector<std::string_view> ParseRoute(const std::vector<std::string>& stops); //Используется при построении справочника
    svg::Color GetColorFromJson(const json::Node& color);
    RenderSettings ParseRenderSettings(const json::Node& catalogue_data);
    std::vector<std::string_view> MakeStopsNamesVector(const std::vector<json::Node>& stops); //Используется при отрисовки карты
}
  
std::map<std::string_view, RouteInfo> GetAllBuses(const TransportCatalogue& tansport_catalogue, const json::Node& catalogue_data); //Используется при отрисовки карты  
json::Document ParseAndMakeAnswers(const TransportCatalogue& tansport_catalogue, const json::Node& catalogue_data);
void LoadCatalogueFromJson(TransportCatalogue& catalogue, const json::Node& root);
    
}