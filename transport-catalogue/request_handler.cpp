#include "request_handler.h"

namespace request_handler
{

RequestHandler::RequestHandler(TransportCatalogue& db, const MapRenderer& renderer) : db_(db),
    map_renderer_(renderer)
{
}


BusStat RequestHandler::GetAllInfoAboutBus(Bus* bus, TransportCatalogue& catalogue)
{

    return catalogue.GetAllInfoAboutBus(bus);
}


detail::BusStat RequestHandler::GetBusStat(TransportCatalogue& catalogue, std::string_view bus_name)
{

    return catalogue.GetBusStat(bus_name);
}


detail::StopStat RequestHandler::StopQuery(TransportCatalogue& catalogue, std::string_view stop_name)
{
    return catalogue.StopQuery(stop_name);
}


std::vector<geo::Coordinates> RequestHandler::GetStopCoordinates() const
{
    return db_.GetStopCoordinates();
}


void RequestHandler::BuildingMap(std::ostream& out) const
{

    //buses == BusMap == std::unordered_map<std::string_view, Bus*>;
    //маршруты, которые будем выводить(в неотсортированном порядке)
    BusMap buses = db_.GetBusNames();

    //сортируем маршруты, которые будем выводить
    std::map sorted_buses(buses.begin(), buses.end());

    //сет будет содержать отсортированные имена остановок
    std::vector<Stop*> stops_ptrs;

    auto stops = db_.GetStopNames();
    std::map sorted_stops(stops.begin(), stops.end());


    //заполняем сет названиями остановок
    for (auto& [stop_name, stop] : sorted_stops)
    {
        if (stop->buses.size() > 0)
        {
            stops_ptrs.push_back(stop);
        }
    }


    std::vector<geo::Coordinates> coordinates = GetStopCoordinates();

    SphereProjector sphere_projector = map_renderer_.GetSphereProjector(coordinates);

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
            map_renderer_.BuildingMapAddLine(buses_palette, doc, sphere_projector);
            map_renderer_.BuildingMapAddBusesNames(buses_palette, doc, sphere_projector);
        }
    }

    map_renderer_.BuildingMapAddStopsCircles(doc, sphere_projector, stops_ptrs);
    map_renderer_.BuildingMapAddStopsNames(doc, sphere_projector, stops_ptrs);

    doc.Render(out);
}


void RequestHandler::RenderMap(std::ostream& out) const
{
    BuildingMap(out);
}

}//end namespace request_handler
