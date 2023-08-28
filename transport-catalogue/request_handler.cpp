#include "request_handler.h"

namespace request_handler
{

RequestHandler::RequestHandler(TransportCatalogue& db, const MapRenderer& renderer) : db_(db),
    map_renderer_(renderer)
{
}


BusStat RequestHandler::GetAllInfoAboutBus(Bus* bus, TransportCatalogue& catalogue)
{
    BusStat bus_stat;

    bus_stat.name = bus->name;
    bus_stat.not_found = false;
    bus_stat.stops_on_route = bus->stops.size();
    bus_stat.unique_stops = catalogue.GetUniqueStopsForBus(bus).size();
    bus_stat.route_length = bus->route_length;
    bus_stat.curvature = double(catalogue.GetDistanceForBus(bus) / catalogue.GetLength(bus));

    return bus_stat;
}


detail::BusStat RequestHandler::GetBusStat(TransportCatalogue& catalogue, std::string_view bus_name)
{
    detail::BusStat bus_stat;

    Bus* bus = catalogue.GetBus(bus_name);
    if (bus != nullptr)
    {

        bus_stat = GetAllInfoAboutBus(bus, catalogue);


    }
    else
    {
        bus_stat.name = bus_name;
        bus_stat.not_found = true;
    }

    return bus_stat;
}


detail::StopStat RequestHandler::StopQuery(TransportCatalogue& catalogue, std::string_view stop_name)
{
    std::set<std::string> unique_buses;

    detail::StopStat stop_info;

    Stop* stop = catalogue.GetStop(stop_name);

    if (stop != NULL)
    {

        stop_info.name = stop->name;
        stop_info.not_found = false;
        unique_buses = catalogue.GetUniqueBusesForStop(stop);

        if (unique_buses.size() > 0)
        {

            for (const auto bus : unique_buses)
            {
                stop_info.buses_name.push_back(bus);
            }

            std::sort(stop_info.buses_name.begin(), stop_info.buses_name.end());
        }

    }
    else
    {
        stop_info.name = stop_name;
        stop_info.not_found = true;
    }

    return stop_info;
}


std::vector<geo::Coordinates> RequestHandler::GetStopCoordinates() const
{

    std::vector<geo::Coordinates> stops_coordinates;
    auto buses = db_.GetBusNames();

    for (auto& [busname, bus] : buses)
    {
        for (auto& stop : bus->stops)
        {
            geo::Coordinates coordinates;
            coordinates.latitude = stop->coordinates.latitude;
            coordinates.longitude = stop->coordinates.longitude;

            stops_coordinates.push_back(coordinates);
        }
    }
    return stops_coordinates;
}


void RequestHandler::BuildingMapAddLine(std::vector<std::pair<Bus*, int>>& buses_palette, svg::Document& doc, SphereProjector& sphere_projector) const
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
            map_renderer_.ConstructPolyline(bus_line, color);
            doc.Add(bus_line);
        }
        stops_coordinates.clear();
    }
}


void RequestHandler::BuildingMapAddBusesNames(std::vector<std::pair<Bus*, int>>& buses_palette, svg::Document& doc, SphereProjector& sphere_projector) const
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
                map_renderer_.ConstructBusTextSubstrate(first_stop_substrate, std::string(bus->name), sphere_projector(stops_coordinates[0]));
                doc.Add(first_stop_substrate);

                //конечная остановка кольцевого
                map_renderer_.ConstructBusText(first_stop, std::string(bus->name), palette, sphere_projector(stops_coordinates[0]));
                doc.Add(first_stop);
            }
            else
            {
                svg::Text first_stop_substrate;
                svg::Text first_stop;
                svg::Text second_stop_substrate;
                svg::Text last_stop;

                //подложка для начальной остановки некольцевого
                map_renderer_.ConstructBusTextSubstrate(first_stop_substrate, std::string(bus->name), sphere_projector(stops_coordinates[0]));
                doc.Add(first_stop_substrate);

                //название для начальной остановки некольцевого
                map_renderer_.ConstructBusText(first_stop, std::string(bus->name), palette, sphere_projector(stops_coordinates[0]));
                doc.Add(first_stop);

                //конечная остановка посередине маршрута
                if (stops_coordinates[0] != stops_coordinates[stops_coordinates.size() / 2])
                {

                    //подложка для конечной остановки некольцевого
                    map_renderer_.ConstructBusTextSubstrate(second_stop_substrate, std::string(bus->name), sphere_projector(stops_coordinates[stops_coordinates.size() / 2]));
                    doc.Add(second_stop_substrate);

                    //название для конечной остановки некольцевого
                    map_renderer_.ConstructBusText(last_stop, std::string(bus->name), palette, sphere_projector(stops_coordinates[stops_coordinates.size() / 2]));
                    doc.Add(last_stop);
                }
            }
        }

        bus_empty = false;
        stops_coordinates.clear();
    }
}


void RequestHandler::BuildingMapAddStopsCircles(svg::Document& doc, SphereProjector& sphere_projector, std::set<std::string_view>& stops_name) const
{

    Stop* stop_ptr;
    svg::Circle circle;

    for (std::string_view stop_name : stops_name)
    {
        stop_ptr = db_.GetStop(stop_name);

        if (stop_ptr)
        {
            geo::Coordinates coordinates;
            coordinates.latitude = stop_ptr->coordinates.latitude;
            coordinates.longitude = stop_ptr->coordinates.longitude;

            map_renderer_.ConstructCircle(circle, sphere_projector(coordinates));
            doc.Add(circle);
        }
    }
}


void RequestHandler::BuildingMapAddStopsNames(svg::Document& doc, SphereProjector& sphere_projector, std::set<std::string_view>& stops_name) const
{
    Stop* stop_ptr;

    svg::Text stop_name_substrate;
    svg::Text stop_name;

    for (std::string_view name : stops_name)
    {
        stop_ptr = db_.GetStop(name);

        if (stop_ptr)
        {
            geo::Coordinates coordinates;
            coordinates.latitude = stop_ptr->coordinates.latitude;
            coordinates.longitude = stop_ptr->coordinates.longitude;

            //подложка для названия остановки
            map_renderer_.ConstructStopTextSubstrate(stop_name_substrate,
                    stop_ptr->name, sphere_projector(coordinates));
            doc.Add(stop_name_substrate);

            //название остановки
            map_renderer_.ConstructStopText(stop_name,
                                            stop_ptr->name, sphere_projector(coordinates));
            doc.Add(stop_name);
        }
    }
}


void RequestHandler::BuildingMap(std::ostream& out) const
{

    //buses == BusMap == std::unordered_map<std::string_view, Bus*>;
    //маршруты, которые будем выводить(в неотсортированном порядке)
    BusMap buses = db_.GetBusNames();

    //сортируем маршруты, которые будем выводить
    std::map sorted_buses(buses.begin(), buses.end());

    //сет будет содержать отсортированные имена остановок
    std::set<std::string_view> stops_name;

    auto stops = db_.GetStopNames();

    //заполняем сет названиями остановок
    for (auto& [stop_name, stop] : stops)
    {
        if (stop->buses.size() > 0)
        {
            stops_name.insert(stop_name);
        }
    }

    //проэцируем координаты остановок на плоскость
    SphereProjector sphere_projector = map_renderer_.GetSphereProjector(GetStopCoordinates());

    svg::Document doc;

    //палитра цветов для маршрутов
    std::vector<std::pair<Bus*, int>> buses_palette;

    int palette_size = 0;
    int palette_index = 0;

    //получаем колличество цветов в палитре из настроек map_rendere
    palette_size = map_renderer_.GetPaletteSize();
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

    BuildingMapAddStopsCircles(doc, sphere_projector, stops_name);
    BuildingMapAddStopsNames(doc, sphere_projector, stops_name);

    doc.Render(out);
}


void RequestHandler::RenderMap(std::ostream& out) const
{
    BuildingMap(out);
}

}//end namespace request_handler
