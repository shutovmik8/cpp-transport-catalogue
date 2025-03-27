#include "json_reader.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "graph.h"
#include "domain.h"

#include <chrono>
#include <iostream>
#include <string>

using namespace std;

int main() {
    transport_catalogue::TransportCatalogue catalogue;
    json::Document catalogue_data{json::Load(std::cin)};
    transport_catalogue::LoadCatalogueFromJson(catalogue, catalogue_data.GetRoot());
    json::Document answer_data{transport_catalogue::ParseAndMakeAnswers(catalogue, catalogue_data.GetRoot())};
    json::Print(answer_data, std::cout);;
}