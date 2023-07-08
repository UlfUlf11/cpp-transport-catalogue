#pragma once

// класс транспортного справочника;

#include <deque>
#include <string>
#include <vector>
#include <iomanip>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <execution>

#include "geo.h"

namespace transport_catalogue {

struct Bus;

struct Stop {
    std::string name;
    double latitude;
    double longitude;
    //вектор хранит увказатели на все автобусы, проезжающие остановку
    std::vector<Bus*> buses;
};

struct Bus {
    std::string name;
    std::deque<Stop*> stops;
};

struct Distance {
    const Stop* A;
    const Stop* B;
    int distance;
};

class DistanceHasher {
public:
    std::size_t operator()(const std::pair<const Stop*, const Stop*> pair_stops) const {
        //нам подходит указатель на неопределенный тип
        auto hash_1 = static_cast<const void*>(pair_stops.first);
        auto hash_2 = static_cast<const void*>(pair_stops.second);
        return std::hash<const void*> {} (hash_1) * 37 + std::hash<const void*> {} (hash_2);
    }
};

using StopMap = std::unordered_map<std::string_view, Stop*>;
using BusMap = std::unordered_map<std::string_view, Bus*>;
using DistanceMap = std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistanceHasher>;

class TransportCatalogue {
public:
    void AddBus(Bus&& bus);
    void AddStop(Stop&& stop);
    void AddDistance(std::vector<Distance> distances);

    Bus* GetBus(std::string_view stop);
    Stop* GetStop(std::string_view stop);

    size_t GetUniqueStopsForBus(const Bus* bus);

    std::unordered_set<const Bus*> GetUniqueBusesForStop(Stop* stop);

    double GetLength(Bus* bus);

    size_t GetDistanceForStop(const Stop* point_a, const Stop* point_b);
    
    size_t GetDistanceForBus(Bus* bus);

private:
    std::deque<Stop> stops_;
    //Если в качестве ключа использовать указатель, подойдёт стандартный хешер.
    StopMap stop_names_;

    std::deque<Bus> buses_;
    //Если в качестве ключа использовать указатель, подойдёт стандартный хешер.
    BusMap bus_names_;

    DistanceMap distance_beetween_stops_;
};

}//завершаем пространство имён transport_catalogue
