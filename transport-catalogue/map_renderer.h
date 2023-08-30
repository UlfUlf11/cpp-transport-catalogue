#pragma once
#include <iostream>
#include <optional>
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <map>

#include "domain.h"
#include "geo.h"
#include "svg.h"

using namespace transport_catalogue::detail;

namespace map_renderer
{

inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector
{
public:

    template <typename InputIt>
    SphereProjector(InputIt points_begin,
                    InputIt points_end,
                    double max_width,
                    double max_height,
                    double padding);

    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

//структура, которая хранит настройки визуализации карты
struct RenderSettings
{
    double width_;
    double height_;
    double padding_;
    double line_width_;
    double stop_radius_;
    int bus_label_font_size_;
    std::pair<double, double> bus_label_offset_;
    int stop_label_font_size_;
    std::pair<double, double>  stop_label_offset_;
    svg::Color underlayer_color_;
    double underlayer_width_;
    std::vector<svg::Color> color_palette_;
};

class MapRenderer
{
public:
    MapRenderer(RenderSettings& render_settings);

    SphereProjector GetSphereProjector(const std::vector<geo::Coordinates>& points) const;

    RenderSettings GetRenderSettings() const;

    int GetPaletteSize() const;

    svg::Color GetColor(int line_number) const;

    //cледующие функции помогают выводить объекты svg вместе с настройками render_settings_
    void ConstructPolyline(svg::Polyline& polyline, int line_number) const;
    void ConstructBusTextSubstrate(svg::Text& text, const std::string& name, svg::Point position) const;
    void ConstructBusText(svg::Text& text, const std::string& name, int palette, svg::Point position) const;
    void ConstructCircle(svg::Circle& circle, svg::Point position) const;
    void ConstructStopTextSubstrate(svg::Text& text, const std::string& name, svg::Point position) const;
    void ConstructStopText(svg::Text& text, const std::string& name, svg::Point position) const;

    void BuildingMapAddLine(std::vector<std::pair<Bus*, int>>& buses_palette, svg::Document& doc, SphereProjector& sphere_projector) const;
    void BuildingMapAddBusesNames(std::vector<std::pair<Bus*, int>>& buses_palette, svg::Document& doc, SphereProjector& sphere_projector) const;
    void BuildingMapAddStopsCircles(svg::Document& doc, SphereProjector& sphere_projector, std::vector<Stop*> stops_ptrs) const;
    void BuildingMapAddStopsNames(svg::Document& doc, SphereProjector& sphere_projector, std::vector<Stop*> stops_ptrs) const;


private:
    RenderSettings& render_settings_;
};

template <typename InputIt>
SphereProjector::SphereProjector(InputIt points_begin,
                                 InputIt points_end,
                                 double max_width,
                                 double max_height,
                                 double padding) : padding_(padding)
{
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end)
        {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
                                             points_begin, points_end,
                                             [](auto lhs, auto rhs)
        {
            return lhs.longitude < rhs.longitude;
        });
        min_lon_ = left_it->longitude;
        const double max_lon = right_it->longitude;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
                                             points_begin, points_end,
                                             [](auto lhs, auto rhs)
        {
            return lhs.latitude < rhs.latitude;
        });
        const double min_lat = bottom_it->latitude;
        max_lat_ = top_it->latitude;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_))
        {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat))
        {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom)
        {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom)
        {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom)
        {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }
}

}//end namespace map_renderer
