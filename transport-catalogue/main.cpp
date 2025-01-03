/*
    * Примерная структура программы:
    *
    * Считать JSON из stdin
    * Построить на его основе JSON базу данных транспортного справочника
    * Выполнить запросы к справочнику, находящиеся в массива "stat_requests", построив JSON-массив
    * с ответами Вывести в stdout ответы в виде JSON
*/

#include <iostream>
#include <string>

#include "json_reader.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

using namespace std;

int main() {
    transport_catalogue::TransportCatalogue catalogue;
    json::Document catalogue_data{json::Load(std::cin)};
    transport_catalogue::LoadCatalogueFromJson(catalogue, catalogue_data.GetRoot());
    json::Document answer_data{transport_catalogue::ParseAndMakeAnswers(catalogue, catalogue_data.GetRoot())};
    json::Print(answer_data, std::cout);
}