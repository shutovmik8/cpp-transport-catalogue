#pragma once

#include "geo.h"
#include "svg.h"
#include "domain.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <map>
#include <utility>


namespace transport_catalogue {

inline const double EPSILON = 1e-6;

struct RenderSettings {
    double width = 0.;
    double height = 0.;
    double padding = 0.;
    double stop_radius = 0.;
    double line_width = 0.;
    int bus_label_font_size = 0;
    double bus_label_offset[2] = {0., 0.};
    int stop_label_font_size = 0;
    double stop_label_offset[2] = {0., 0.};
    svg::Color underlayer_color;
    double underlayer_width = 0.;
    std::vector<svg::Color> color_palette;
};
   
bool IsZero(double value);

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        //std::cout << "min_lng: " << left_it->lng << "max_lng: " << right_it->lng << std::endl;
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        //std::cout << "min_lat: " << bottom_it->lat << "max_lat: " << top_it->lat << std::endl;
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }
        //std::cout << "width_zoom: " << width_zoom.value() << std::endl;
        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }
        //std::cout << "height_zoom: " << height_zoom.value() << std::endl;
        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
        //std::cout << zoom_coeff_ << " " << padding_ << std::endl;
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(transport_catalogue::Coordinates coords) const;

private:
    double padding_;
    double min_lon_{0};
    double max_lat_{0};
    double zoom_coeff_{0};
}; 

//Заменяет координаты на поверхности земли (lat, lng) на координаты на плоскости (x, y)
std::map<std::string_view, RouteInfo>  ProjectionStopsPoints(const RenderSettings& settings, const std::map<std::string_view, RouteInfo>& buses);
    
void PrintRoutesLines(const RenderSettings& settings, const std::map<std::string_view, RouteInfo>& buses, svg::Document& doc);
       
void PrintRoutesNames(const RenderSettings& settings, const std::map<std::string_view, RouteInfo>& buses, svg::Document& doc);

//Возвращает все имеющиеся остановки, отсортированные в алфавитном порядке
std::map<std::string_view, svg::Point> GetAllSortStops(const std::map<std::string_view, RouteInfo>& buses);
    
void PrintStopsCircles(const RenderSettings& settings, svg::Document& doc, const std::map<std::string_view, svg::Point>& stops);
 
void PrintStopsNames(const RenderSettings& settings, svg::Document& doc, const std::map<std::string_view, svg::Point>& stops);
    
std::string GetMapJson(const RenderSettings& settings, const std::map<std::string_view, RouteInfo>& buses);
   
} //transport_catalogue