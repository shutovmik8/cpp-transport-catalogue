#include "map_renderer.h"

#include <set>
#include <sstream>

namespace transport_catalogue {

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Point SphereProjector::operator()(Coordinates coords) const {
    return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
}

std::map<std::string_view, RouteInfo> ProjectionStopsPoints(const RenderSettings& settings, const std::map<std::string_view, RouteInfo>& buses) {
    std::set<Coordinates> coordinates;
    std::map<std::string_view, RouteInfo> buses_ = buses;
    for (const auto& bus : buses) {
        for (const auto& stop : bus.second.stops) {
            coordinates.insert({stop.coordinates.lat, stop.coordinates.lng});
        }
    }
    const SphereProjector proj{coordinates.begin(), coordinates.end(), settings.width, settings.height, settings.padding};
    for (auto& bus : buses_) {
        for (auto& stop : bus.second.stops) {
            svg::Point point{proj({stop.coordinates.lat, stop.coordinates.lng})};
            stop.coordinates.lat = point.x;
            stop.coordinates.lng = point.y;
        }
    }
    return buses_;
}
    
void PrintRoutesLines(const RenderSettings& settings, const std::map<std::string_view, RouteInfo>& buses, svg::Document& doc) {
    int color_num{0};
    size_t color_palette_size{settings.color_palette.size()};
    for (const auto& bus : buses) {
        if (bus.second.stops.size() < 3) {
            continue;
        }
        svg::Polyline polyline{};
        for (const auto& stop : bus.second.stops) {
            polyline.AddPoint({stop.coordinates.lat, stop.coordinates.lng});
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
    
void PrintRoutesNames(const RenderSettings& settings, const std::map<std::string_view, RouteInfo>& buses, svg::Document& doc) {
    int color_num{0};
    size_t color_palette_size{settings.color_palette.size()};
    for (const auto& bus : buses) {
        if (bus.second.stops.size() < 3) {
            continue;
        }
        svg::Text bus_name{};
        bus_name.SetPosition({bus.second.stops.at(0).coordinates.lat, bus.second.stops.at(0).coordinates.lng})
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
        if ((!bus.second.is_roundtrip) and (bus.second.stops.at(0).name != bus.second.stops.at(bus.second.stops.size() / 2).name)) {
            
            doc.Add(svg::Text{bus_name}.SetPosition({bus.second.stops.at(bus.second.stops.size() / 2).coordinates.lat, bus.second.stops.at(bus.second.stops.size() / 2).coordinates.lng})
            .SetFillColor(settings.underlayer_color)
            .SetStrokeColor(settings.underlayer_color)
            .SetStrokeWidth(settings.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
            
            doc.Add(svg::Text{bus_name}.SetPosition({bus.second.stops.at(bus.second.stops.size() / 2).coordinates.lat, bus.second.stops.at(bus.second.stops.size() / 2).coordinates.lng})
                    .SetFillColor(settings.color_palette[color_num % color_palette_size]));
        }
        
        ++color_num;
    }    
}

std::map<std::string_view, svg::Point> GetAllSortStops(const std::map<std::string_view, RouteInfo>& buses) {
    std::map<std::string_view, svg::Point> stops;
    for (const auto& bus : buses) {
        for (const auto& stop : bus.second.stops) {
            if (!stops.contains(stop.name)) {
                stops.insert({stop.name, {stop.coordinates.lat, stop.coordinates.lng}});
            }
        }   
    }
    return stops;
}
    
void PrintStopsCircles(const RenderSettings& settings, svg::Document& doc, const std::map<std::string_view, svg::Point>& stops) {
    svg::Circle circle = svg::Circle().SetRadius(settings.stop_radius)
        .SetFillColor("white");
    for (const auto& stop : stops) {
        doc.Add(svg::Circle{circle}.SetCenter(stop.second));
    }
}

void PrintStopsNames(const RenderSettings& settings, svg::Document& doc, const std::map<std::string_view, svg::Point>& stops) {
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
    
std::string GetMapJson(const RenderSettings& settings, const std::map<std::string_view, RouteInfo>& buses) {
    svg::Document doc;
    std::vector<Bus> = 
    std::map<std::string_view, RouteInfo> buses_with_points = ProjectionStopsPoints(settings, buses);
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