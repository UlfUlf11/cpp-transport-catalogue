// класс транспортного справочника;

#include "transport_catalogue.h"

namespace transport_catalogue {

void TransportCatalogue::AddStop(Stop&& stop) {
    stops_.push_back(std::move(stop));

    //указатель на добавленную остановку
    Stop* stop_ptr = &stops_.back();

    //≈сли в качестве ключа использовать указатель, подойдЄт стандартный хешер.
    //StopMap = std::unordered_map<std::string_view, Stop*>;
    stop_names_.insert(StopMap::value_type(stop_ptr->name, stop_ptr));
}

void TransportCatalogue::AddBus(Bus&& bus) {
    buses_.push_back(std::move(bus));

    //указатель на добавленный автобус
    Bus* bus_ptr = &buses_.back();

    //≈сли в качестве ключа использовать указатель, подойдЄт стандартный хешер.
    //BusMap = std::unordered_map<std::string_view, Bus*>;
    bus_names_.insert(BusMap::value_type(bus_ptr->name, bus_ptr));

    for (Stop* stop : bus_ptr->stops) {
        stop->buses.push_back(bus_ptr);
    }
}

void TransportCatalogue::AddDistance(std::vector<Distance> distances) {
    for (auto distance : distances) {
        auto stops_pair = std::make_pair(distance.A, distance.B);
        
        //DistanceMap = std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistanceHasher>;
        distance_beetween_stops_.insert(DistanceMap::value_type(stops_pair, distance.distance));
    }
}

Bus* TransportCatalogue::GetBus(std::string_view bus) {
    if (bus_names_.empty()) {
        return nullptr;
    }
    try {
        return bus_names_.at(bus);
    }
    catch (const std::out_of_range& error) {
        return nullptr;
    }
}

Stop* TransportCatalogue::GetStop(std::string_view stop) {
    if (stop_names_.empty()) {
        return nullptr;
    }
    try {
        return stop_names_.at(stop);
    }
    catch (const std::out_of_range& error) {
        return nullptr;
    }
}

//возвращает кол-во уникальных остановок дл€ маршрута
size_t TransportCatalogue::GetUniqueStopsForBus(const Bus* bus) {
    std::unordered_set<const Stop*> unique_stops;

    unique_stops.insert(bus->stops.begin(), bus->stops.end());

    return unique_stops.size();
}

std::unordered_set<const Bus*> TransportCatalogue::GetUniqueBusesForStop(Stop* stop) {
    std::unordered_set<const Bus*> unique_stops;

    unique_stops.insert(stop->buses.begin(), stop->buses.end());

    return unique_stops;
}

//вычисл€ет географическое рассто€ние
double TransportCatalogue::GetLength(Bus* bus) {
    //берем все остановки от 2й до последней...
    return transform_reduce(next(bus->stops.begin()), bus->stops.end(),
        //... и прибавл€ем их дистанции к первой остановке
        bus->stops.begin(),
        0.0,
        std::plus<>{},
        [](const Stop* lhs, const Stop* rhs) {
            return geo::ComputeDistance({ (*lhs).latitude, (*lhs).longitude }, 
                                   { (*rhs).latitude, (*rhs).longitude });
        });
}

size_t TransportCatalogue::GetDistanceForStop(const Stop* point_a, const Stop* point_b) {
    if (distance_beetween_stops_.empty()) {
        return 0;
    }
    try {
        auto stops_pair = std::make_pair(point_a, point_b);
        return distance_beetween_stops_.at(stops_pair);
    }
    catch (const std::out_of_range& error) {
        try {
            auto stops_pair = std::make_pair(point_b, point_a);
            return distance_beetween_stops_.at(stops_pair);
        }
        catch (const std::out_of_range& error) {
            return 0;
        }
    }
}

size_t TransportCatalogue::GetDistanceForBus(Bus* bus) {
    size_t distance = 0;
    auto stops_size = bus->stops.size() - 1;
    for (int i = 0; i < stops_size; i++) {
        distance += GetDistanceForStop(bus->stops[i], bus->stops[i + 1]);
    }
    return distance;
}

}//завершаем пространство имЄн transport_catalogue