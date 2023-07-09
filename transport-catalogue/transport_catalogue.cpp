// класс транспортного справочника;

#include "transport_catalogue.h"

namespace transport_catalogue {

void TransportCatalogue::AddStop(const Stop&& stop) {
    stops_.push_back(std::move(stop));

    //указатель на добавленную остановку
    Stop* stop_ptr = &stops_.back();

    //≈сли в качестве ключа использовать указатель, подойдЄт стандартный хешер.
    //StopMap = std::unordered_map<std::string_view, Stop*>;
    stop_names_.insert(StopMap::value_type(stop_ptr->name, stop_ptr));
}

void TransportCatalogue::AddBus(const Bus&& bus) {
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

void TransportCatalogue::AddDistance(const Stop* stop_a, const Stop* stop_b, int distance) {

        auto stops_pair = std::make_pair(stop_a, stop_b);
        
        //DistanceMap = std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistanceHasher>;
        distance_beetween_stops_.insert(DistanceMap::value_type(stops_pair, distance));
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
std::set<const Stop*> TransportCatalogue::GetUniqueStopsForBus(const Bus* bus) {
    std::set<const Stop*> unique_stops_sorted;
    std::deque<Stop*>  all_stops = GetAllStopsForBus(bus);

    unique_stops_sorted.insert(all_stops.begin(), all_stops.end());

    return unique_stops_sorted;
}

std::set<std::string> TransportCatalogue::GetUniqueBusesForStop(Stop* stop) {
    std::set<std::string> unique_buses;
    std::vector<Bus*> all_buses = GetAllBusesForStop(stop);

    for (auto bus : all_buses) {
        unique_buses.insert(bus->name);
    }

    return unique_buses;
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
            return geo::ComputeDistance({ (*lhs).coordinates.lat, (*lhs).coordinates.lng },
                                   { (*rhs).coordinates.lat, (*rhs).coordinates.lng });
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



std::deque<Stop*> TransportCatalogue::GetAllStopsForBus(const Bus* bus) {
    return  bus->stops;
}

std::vector<Bus*> TransportCatalogue::GetAllBusesForStop(Stop* stop) {
    return stop->buses;
}

}//завершаем пространство имЄн transport_catalogue