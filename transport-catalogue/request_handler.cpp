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





    void RequestHandler::RenderMap(std::ostream& out, TransportCatalogue& catalogue) const
    {
        map_renderer_.BuildingMap(out, catalogue);
    }

}//end namespace request_handler
