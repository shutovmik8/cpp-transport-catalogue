#include <algorithm>
#include <cassert>
#include <iterator>
#include <iostream>

#include "input_reader.h"

namespace trancport_catalogue {

namespace detail {    
/**
 * Парсит строку вида "10.123,  -30.1837, D1m to stop1, D2m to stop2, ..." и возвращает unordered_map<name, distance>
 */    
std::unordered_map<std::string, int> ParseDistances(std::string_view str) {
    std::unordered_map<std::string, int> distances;
    auto comma = str.find(',');
    comma = str.find(',', comma + 1);
    if (comma == str.npos) {
        return distances;
    }
    while(1) {
        auto first_number = str.find_first_not_of(' ', comma + 1);
        auto litera_m = str.find('m', first_number);
        int len = std::stoi(std::string(str.substr(first_number, litera_m - first_number)));
        auto first_name_symbol = str.find('o', litera_m + 1);
        first_name_symbol = str.find_first_not_of(' ', first_name_symbol + 1);
        comma = str.find(',', first_name_symbol);
        if (comma == str.npos) {
            distances[std::string(str.substr(first_name_symbol))] = len;
            break;
        }
        else {
            distances[std::string(str.substr(first_name_symbol, comma - first_name_symbol))] = len;
        }
    }
    return distances;
}   
/**
 * Парсит строку вида "10.123,  -30.1837, D1m to stop1, D2m to stop2, ..." и возвращает пару координат (широта, долгота)
 */
Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);
    auto comma2 = str.find(',', not_space2);
    
    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng;
    if (comma2 == str.npos) {
        lng = std::stod(std::string(str.substr(not_space2)));
    }
    else {
        lng = std::stod(std::string(str.substr(not_space2, comma2 - not_space2)));
    }

    return {lat, lng};
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = detail::ParseCommandDescription(line);
    if (command_description) {
        command_description.command = detail::Trim(command_description.command);
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
    for (auto& command_ : commands_) {
        if (command_.command == "Stop") {
            catalogue.AddStop(command_.id, detail::ParseCoordinates(command_.description));
        }
    }
    for (auto& command_ : commands_) {
        if (command_.command == "Stop") {
            for (auto& item : detail::ParseDistances(command_.description)) {
                catalogue.AddDistance(std::move(command_.id), item.first, item.second);
            }
        }
    }
    for (auto& command_ : commands_) {
        if (command_.command == "Bus") {
            catalogue.AddBus(std::move(command_.id), detail::ParseRoute(command_.description));
        }
        
    }
}

}