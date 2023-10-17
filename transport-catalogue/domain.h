#pragma once

#include <algorithm>
#include <vector>
#include <deque>
#include <string>
#include "geo.h"

namespace transport_catalogue
{
    namespace detail
    {
        using namespace transport_catalogue;

        struct Bus;

        struct Stop
        {
            std::string name;
            detail::geo::Coordinates coordinates;

            //вектор хранит увказатели на все автобусы, проезжающие остановку
            std::vector<Bus*> buses;
        };

        struct Bus
        {
            std::string name;
            std::deque<Stop*> stops;
            bool is_roundtrip;
            size_t route_length;
        };

        //структура для возврата данных по автобусу
        struct BusStat
        {
            double curvature;
            std::string_view name;
            bool not_found;
            int stops_on_route;
            int unique_stops;
            int route_length;
        };

        //структура для возврата данных по остановке
        struct StopStat {
            std::string_view name;
            bool not_found;
            std::vector<std::string> buses_name;
        };

        struct RoutingSettings {
            double bus_wait_time = 0;
            double bus_velocity = 0;
        };

    } // end namespace detail

} // end namespace transport_catalogue
