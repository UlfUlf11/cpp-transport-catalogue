#include "map_renderer.h"

namespace map_renderer
{


    void MapRenderer::BuildingMapAddLine(std::vector<std::pair<Bus*, int>>& buses_palette, svg::Document& doc, SphereProjector& sphere_projector) const
    {
        std::vector<geo::Coordinates> stops_coordinates;

        for (auto [bus, color] : buses_palette)
        {
            for (auto& stop : bus->stops)
            {
                geo::Coordinates coordinates;
                coordinates.latitude = stop->coordinates.latitude;
                coordinates.longitude = stop->coordinates.longitude;

                stops_coordinates.push_back(coordinates);
            }
            svg::Polyline bus_line;
            bool bus_empty = true;

            for (auto& coordinates : stops_coordinates)
            {
                bus_empty = false;
                bus_line.AddPoint(sphere_projector(coordinates));
            }

            if (!bus_empty)
            {
                ConstructPolyline(bus_line, color);
                doc.Add(bus_line);
            }
            stops_coordinates.clear();
        }
    }


    void MapRenderer::BuildingMapAddBusesNames(std::vector<std::pair<Bus*, int>>& buses_palette, svg::Document& doc, SphereProjector& sphere_projector) const
    {

        //вектор координат остановок
        std::vector<geo::Coordinates> stops_coordinates;

        //флаг чтобы добавлять координаты только один раз
        bool bus_empty = true;

        for (auto [bus, palette] : buses_palette)
        {
            for (auto& stop : bus->stops)
            {
                geo::Coordinates coordinates;
                coordinates.latitude = stop->coordinates.latitude;
                coordinates.longitude = stop->coordinates.longitude;

                stops_coordinates.push_back(coordinates);

                if (bus_empty) bus_empty = false;
            }

            if (!bus_empty)
            {
                if (bus->is_roundtrip)
                {
                    svg::Text first_stop_substrate;
                    svg::Text first_stop;

                    //подложка для кольцевого
                    ConstructBusTextSubstrate(first_stop_substrate, std::string(bus->name), sphere_projector(stops_coordinates[0]));
                    doc.Add(first_stop_substrate);

                    //конечная остановка кольцевого
                    ConstructBusText(first_stop, std::string(bus->name), palette, sphere_projector(stops_coordinates[0]));
                    doc.Add(first_stop);
                }
                else
                {
                    svg::Text first_stop_substrate;
                    svg::Text first_stop;
                    svg::Text second_stop_substrate;
                    svg::Text last_stop;

                    //подложка для начальной остановки некольцевого
                    ConstructBusTextSubstrate(first_stop_substrate, std::string(bus->name), sphere_projector(stops_coordinates[0]));
                    doc.Add(first_stop_substrate);

                    //название для начальной остановки некольцевого
                    ConstructBusText(first_stop, std::string(bus->name), palette, sphere_projector(stops_coordinates[0]));
                    doc.Add(first_stop);

                    //конечная остановка посередине маршрута
                    if (stops_coordinates[0] != stops_coordinates[stops_coordinates.size() / 2])
                    {

                        //подложка для конечной остановки некольцевого
                        ConstructBusTextSubstrate(second_stop_substrate, std::string(bus->name), sphere_projector(stops_coordinates[stops_coordinates.size() / 2]));
                        doc.Add(second_stop_substrate);

                        //название для конечной остановки некольцевого
                        ConstructBusText(last_stop, std::string(bus->name), palette, sphere_projector(stops_coordinates[stops_coordinates.size() / 2]));
                        doc.Add(last_stop);
                    }
                }
            }

            bus_empty = false;
            stops_coordinates.clear();
        }
    }


    void MapRenderer::BuildingMapAddStopsCircles(svg::Document& doc, SphereProjector& sphere_projector, std::vector<Stop*> stops_ptrs) const
    {
        svg::Circle circle;

        for (Stop* stop_ptr : stops_ptrs)
        {
            if (stop_ptr)
            {
                geo::Coordinates coordinates;
                coordinates.latitude = stop_ptr->coordinates.latitude;
                coordinates.longitude = stop_ptr->coordinates.longitude;

                ConstructCircle(circle, sphere_projector(coordinates));
                doc.Add(circle);
            }
        }
    }


    void MapRenderer::BuildingMapAddStopsNames(svg::Document& doc, SphereProjector& sphere_projector, std::vector<Stop*> stops_ptrs) const
    {
        svg::Text stop_name_substrate;
        svg::Text stop_name;

        for (Stop* stop_ptr : stops_ptrs)
        {
            if (stop_ptr)
            {
                geo::Coordinates coordinates;
                coordinates.latitude = stop_ptr->coordinates.latitude;
                coordinates.longitude = stop_ptr->coordinates.longitude;

                //подложка для названия остановки
                ConstructStopTextSubstrate(stop_name_substrate,
                    stop_ptr->name, sphere_projector(coordinates));
                doc.Add(stop_name_substrate);

                //название остановки
                ConstructStopText(stop_name,
                    stop_ptr->name, sphere_projector(coordinates));
                doc.Add(stop_name);
            }
        }
    }


    bool IsZero(double value)
    {
        return std::abs(value) < EPSILON;
    }

    MapRenderer::MapRenderer(RenderSettings& render_settings) : render_settings_(render_settings) {}

    svg::Point SphereProjector::operator()(geo::Coordinates coords) const
    {
        return { (coords.longitude - min_lon_) * zoom_coeff_ + padding_,
                 (max_lat_ - coords.latitude) * zoom_coeff_ + padding_ };
    }

    SphereProjector MapRenderer::GetSphereProjector(const std::vector<geo::Coordinates>& points) const
    {
        return SphereProjector(points.begin(),
            points.end(),
            render_settings_.width_,
            render_settings_.height_,
            render_settings_.padding_);
    }

    RenderSettings MapRenderer::GetRenderSettings() const
    {
        return render_settings_;
    }

    int MapRenderer::GetPaletteSize() const
    {
        return render_settings_.color_palette_.size();
    }

    svg::Color MapRenderer::GetColor(int line_number) const
    {
        return render_settings_.color_palette_[line_number];
    }


    void MapRenderer::ConstructPolyline(svg::Polyline& polyline, [[maybe_unused]] int line_number) const
    {
        using namespace std::literals;

        polyline.SetStrokeColor(GetColor(line_number));
        polyline.SetFillColor("none"s);
        polyline.SetStrokeWidth(render_settings_.line_width_);
        polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    }

    void MapRenderer::ConstructBusTextSubstrate(svg::Text& text, const std::string& name, svg::Point position) const
    {

        using namespace std::literals;

        text.SetPosition(position);
        text.SetOffset({ render_settings_.bus_label_offset_.first, render_settings_.bus_label_offset_.second });
        text.SetFontSize(render_settings_.bus_label_font_size_);
        text.SetFontFamily("Verdana");
        text.SetFontWeight("bold");
        text.SetData(name);
        text.SetFillColor(render_settings_.underlayer_color_);
        text.SetStrokeColor(render_settings_.underlayer_color_);
        text.SetStrokeWidth(render_settings_.underlayer_width_);
        text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    }

    void MapRenderer::ConstructBusText(svg::Text& text, const std::string& name, int palette, svg::Point position) const
    {

        using namespace std::literals;

        text.SetPosition(position);
        text.SetOffset({ render_settings_.bus_label_offset_.first, render_settings_.bus_label_offset_.second });
        text.SetFontSize(render_settings_.bus_label_font_size_);
        text.SetFontFamily("Verdana");
        text.SetFontWeight("bold");
        text.SetData(name);
        text.SetFillColor(GetColor(palette));
    }

    void MapRenderer::ConstructCircle(svg::Circle& circle, svg::Point position) const
    {
        using namespace std::literals;

        circle.SetCenter(position);
        circle.SetRadius(render_settings_.stop_radius_);
        circle.SetFillColor("white");
    }

    void MapRenderer::ConstructStopTextSubstrate(svg::Text& text, const std::string& name, svg::Point position) const
    {
        using namespace std::literals;

        text.SetPosition(position);
        text.SetOffset({ render_settings_.stop_label_offset_.first, render_settings_.stop_label_offset_.second });
        text.SetFontSize(render_settings_.stop_label_font_size_);
        text.SetFontFamily("Verdana");
        text.SetData(name);
        text.SetFillColor(render_settings_.underlayer_color_);
        text.SetStrokeColor(render_settings_.underlayer_color_);
        text.SetStrokeWidth(render_settings_.underlayer_width_);
        text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    }

    void MapRenderer::ConstructStopText(svg::Text& text, const std::string& name, svg::Point position) const
    {
        using namespace std::literals;

        text.SetPosition(position);
        text.SetOffset({ render_settings_.stop_label_offset_.first, render_settings_.stop_label_offset_.second });
        text.SetFontSize(render_settings_.stop_label_font_size_);
        text.SetFontFamily("Verdana");
        text.SetData(name);
        text.SetFillColor("black");
    }


    void MapRenderer::BuildingMap(std::ostream& out, TransportCatalogue& catalogue) const
    {

        //buses == BusMap == std::unordered_map<std::string_view, Bus*>;
        //маршруты, которые будем выводить(в неотсортированном порядке)
        BusMap buses = catalogue.GetBusNames();

        //сортируем маршруты, которые будем выводить
        std::map sorted_buses(buses.begin(), buses.end());

        //сет будет содержать отсортированные имена остановок
        std::vector<Stop*> stops_ptrs;

        auto stops = catalogue.GetStopNames();
        std::map sorted_stops(stops.begin(), stops.end());


        //заполняем сет названиями остановок
        for (auto& [stop_name, stop] : sorted_stops)
        {
            if (stop->buses.size() > 0)
            {
                stops_ptrs.push_back(stop);
            }
        }


        std::vector<geo::Coordinates> coordinates = catalogue.GetStopCoordinates();

        SphereProjector sphere_projector = GetSphereProjector(coordinates);

        svg::Document doc;

        //палитра цветов для маршрутов
        std::vector<std::pair<Bus*, int>> buses_palette;

        int palette_size = 0;
        int palette_index = 0;

        //получаем колличество цветов в палитре из настроек map_rendere
        palette_size = GetPaletteSize();
        if (palette_size == 0)
        {
            std::cout << "палитра цветов пуста";
            return;
        }

        /*
        Карта состоит из четырёх типов объектов.Порядок их вывода в SVG - документ:
            1)ломаные линии маршрутов,
            2)названия маршрутов,
            3)круги, обозначающие остановки,
            4)названия остановок.
            */

        if (sorted_buses.size() > 0)
        {
            //выводим маршруты в лексикографическом порядке
            for (auto& [busname, bus] : sorted_buses)
            {

                if (bus->stops.size() > 0)
                {
                    buses_palette.push_back(std::make_pair(bus, palette_index));
                    palette_index++;

                    if (palette_index == palette_size)
                    {
                        palette_index = 0;
                    }
                }
            }

            if (buses_palette.size() > 0)
            {
                BuildingMapAddLine(buses_palette, doc, sphere_projector);
                BuildingMapAddBusesNames(buses_palette, doc, sphere_projector);
            }
        }

        BuildingMapAddStopsCircles(doc, sphere_projector, stops_ptrs);
        BuildingMapAddStopsNames(doc, sphere_projector, stops_ptrs);

        doc.Render(out);
    }

}//end namespace map_renderer
