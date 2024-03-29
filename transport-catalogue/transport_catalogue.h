#pragma once

// класс транспортного справочника;
#include <vector>
#include <iomanip>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <set>
#include <numeric>

#include "domain.h"

namespace transport_catalogue
{

using namespace transport_catalogue::detail;

class DistanceHasher
{
public:
    std::size_t operator()(const std::pair<const Stop*, const Stop*> pair_stops) const

    {
        //нам подходит указатель на неопределенный тип
        auto hash_1 = static_cast<const void*>(pair_stops.first);
        auto hash_2 = static_cast<const void*>(pair_stops.second);

        return std::hash<const void*> {}(hash_1)*37 + std::hash<const void*> {}(hash_2);
    }
};

using StopMap = std::unordered_map<std::string_view, Stop*>;
using BusMap = std::unordered_map<std::string_view, Bus*>;

using DistanceMap = std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistanceHasher>;

class TransportCatalogue
{
public:
    void AddBus(const Bus&& bus);
    void AddStop(const Stop&& stop);
    void AddDistance(const Stop* stop_a, const Stop* stop_b, int distance);

    Bus* GetBus(std::string_view bus);
    Stop* GetStop(std::string_view stop);

    BusMap GetBusNames() const;
    StopMap GetStopNames() const;

    std::set<const Stop*> GetUniqueStopsForBus(const Bus* bus);
    std::set<std::string> GetUniqueBusesForStop(Stop* stop);

    double GetLength(Bus* bus);

    int GetDistanceForStop(const Stop* point_a, const Stop* point_b);
    int GetDistanceForBus(Bus* bus);

    BusStat GetAllInfoAboutBus(Bus* bus);

    // Возвращает информацию о маршруте (запрос Bus)
    detail::BusStat GetBusStat(std::string_view str);

    // Возвращает маршруты, проходящие через
    detail::StopStat StopQuery(std::string_view stop_name);

    std::vector<geo::Coordinates> GetStopCoordinates() const;

    // 13 спринт
    void AddRouteSettings(const detail::RoutingSettings route_settings);
    size_t GetStopsQuantity();
    const std::deque<Bus>& GetBuses() const;
    double GetVelocity();
    double GetWaitTime();

    std::deque<Stop> GetStops() const;
    DistanceMap GetDistance() const;

    detail::RoutingSettings GetRouteSettings() const;

private:
    std::deque<Stop> stops_;
    //Если в качестве ключа использовать указатель, подойдёт стандартный хешер.
    StopMap stop_names_;

    std::deque<Bus> buses_;
    //Если в качестве ключа использовать указатель, подойдёт стандартный хешер.
    BusMap bus_names_;

    DistanceMap distance_beetween_stops_;

    std::deque<Stop*> GetAllStopsForBus(const Bus* bus);
    std::vector<Bus*> GetAllBusesForStop(Stop* stop);

    double bus_wait_time_; // 13 спринт
    double bus_velocity_; //  13 спринт
};
} //завершаем пространство имён transport_catalogue
