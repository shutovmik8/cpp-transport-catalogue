#include <set>
#include <sstream>

#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace transport_catalogue {

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Point SphereProjector::operator()(Coordinates coords) const {
    return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
}

std::map<std::string_view, RouteInfo> GetStopsPoints(RenderSettings settings, std::map<std::string_view, RouteInfo> buses) {
    std::map<std::string_view, RouteInfo> buses_with_points;
    std::set<Coordinates> coordinates;
    for (const auto& bus : buses) {
        for (const auto& stop : bus.second.stops) {
            coordinates.insert({stop.second.x, stop.second.y});
        }
    }
    const SphereProjector proj{coordinates.begin(), coordinates.end(), settings.width, settings.height, settings.padding};
    for (auto& bus : buses) {
        for (auto& stop : bus.second.stops) {
            svg::Point point{proj({stop.second.x, stop.second.y})};
            stop.second.x = point.x;
            stop.second.y = point.y;
        }
    }
    return buses;
}
    
void PrintRoutesLines(RenderSettings settings, std::map<std::string_view, RouteInfo> buses, svg::Document& doc) {
    int color_num{0};
    size_t color_palette_size{settings.color_palette.size()};
    for (const auto& bus : buses) {
        if (bus.second.stops.size() < 3) {
            continue;
        }
        svg::Polyline polyline{};
        for (const auto& stop : bus.second.stops) {
            polyline.AddPoint({stop.second.x, stop.second.y});
        }
        polyline.SetStrokeColor(settings.color_palette[color_num % color_palette_size])
        .SetFillColor(svg::NoneColor)
        .SetStrokeWidth(settings.line_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        ++color_num;
        doc.Add(polyline);
    }
}
    
void PrintRoutesNames(RenderSettings settings, std::map<std::string_view, RouteInfo> buses, svg::Document& doc) {
    int color_num{0};
    size_t color_palette_size{settings.color_palette.size()};
    for (const auto& bus : buses) {
        if (bus.second.stops.size() < 3) {
            continue;
        }
        svg::Text bus_name{};
        bus_name.SetPosition({bus.second.stops.at(0).second.x, bus.second.stops.at(0).second.y})
        .SetOffset({settings.bus_label_offset[0], settings.bus_label_offset[1]})
        .SetFontSize(settings.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetData(std::string(bus.first));

        doc.Add(svg::Text{bus_name}.SetFillColor(settings.underlayer_color)
        .SetStrokeColor(settings.underlayer_color)
        .SetStrokeWidth(settings.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
        
        doc.Add(svg::Text{bus_name}.SetFillColor(settings.color_palette[color_num % color_palette_size]));
        if ((!bus.second.is_roundtrip) and (bus.second.stops.at(0).first != bus.second.stops.at(bus.second.stops.size() / 2).first)) {
            
            doc.Add(svg::Text{bus_name}.SetPosition({bus.second.stops.at(bus.second.stops.size() / 2).second.x, bus.second.stops.at(bus.second.stops.size() / 2).second.y})
            .SetFillColor(settings.underlayer_color)
            .SetStrokeColor(settings.underlayer_color)
            .SetStrokeWidth(settings.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
            
            doc.Add(svg::Text{bus_name}.SetPosition({bus.second.stops.at(bus.second.stops.size() / 2).second.x, bus.second.stops.at(bus.second.stops.size() / 2).second.y})
                    .SetFillColor(settings.color_palette[color_num % color_palette_size]));
        }
        
        ++color_num;
    }    
}

std::map<std::string_view, svg::Point> GetAllSortStops(std::map<std::string_view, RouteInfo> buses) {
    std::map<std::string_view, svg::Point> stops;
    for (const auto& bus : buses) {
        for (const auto& stop : bus.second.stops) {
            if (!stops.contains(stop.first)) {
                stops.insert({stop.first, {stop.second.x, stop.second.y}});
            }
        }   
    }
    return stops;
}
    
void PrintStopsCircles(RenderSettings settings, svg::Document& doc, std::map<std::string_view, svg::Point> stops) {
    svg::Circle circle = svg::Circle().SetRadius(settings.stop_radius)
        .SetFillColor("white");
    for (const auto& stop : stops) {
        doc.Add(svg::Circle{circle}.SetCenter(stop.second));
    }
}

void PrintStopsNames(RenderSettings settings, svg::Document& doc, std::map<std::string_view, svg::Point> stops) {
    svg::Text stop_name = svg::Text{}.SetOffset({settings.stop_label_offset[0], settings.stop_label_offset[1]})
        .SetFontSize(settings.stop_label_font_size)
        .SetFontFamily("Verdana");
    for (const auto& stop : stops) {
        doc.Add(svg::Text{stop_name}.SetPosition(stop.second)
               .SetData(std::string(stop.first))
               .SetFillColor(settings.underlayer_color)
               .SetStrokeColor(settings.underlayer_color)
               .SetStrokeWidth(settings.underlayer_width)
               .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
               .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
        doc.Add(svg::Text{stop_name}.SetPosition(stop.second)
               .SetData(std::string(stop.first))
               .SetFillColor("black"));
    }
}
    
std::string GetMapJson(RenderSettings settings, std::map<std::string_view, RouteInfo> buses) {
    svg::Document doc;
    std::map<std::string_view, RouteInfo> buses_with_points{GetStopsPoints(settings, buses)};
    PrintRoutesLines(settings, buses_with_points, doc);
    PrintRoutesNames(settings, buses_with_points, doc);
    std::map<std::string_view, svg::Point> stops{GetAllSortStops(buses_with_points)};
    PrintStopsCircles(settings, doc, stops);
    PrintStopsNames(settings, doc, stops);
    std::ostringstream map_json;
    doc.Render(map_json);
    return map_json.str();
}

}