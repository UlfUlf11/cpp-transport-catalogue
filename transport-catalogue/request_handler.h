#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"

using namespace transport_catalogue;
using namespace map_renderer;

namespace request_handler
{
    class RequestHandler
    {

    public:
        RequestHandler(TransportCatalogue& db, const MapRenderer& map_renderer_);

        // Возвращает информацию о маршруте (запрос Bus)
        detail::BusStat GetBusStat(TransportCatalogue& catalogue, std::string_view str);

        // Возвращает маршруты, проходящие через
        detail::StopStat StopQuery(
            TransportCatalogue& catalogue, std::string_view stop_name);

        void RenderMap(std::ostream& out, TransportCatalogue& catalogue) const;

    private:
        TransportCatalogue& db_;

        const MapRenderer& map_renderer_;

        BusStat GetAllInfoAboutBus(Bus* bus, TransportCatalogue& catalogue);

        std::vector<geo::Coordinates> GetStopCoordinates() const;
    };

} // end namespace request_handler