#pragma once

// ����� ������������� �����������;

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
    //������ ������ ���������� �� ��� ��������, ����������� ���������
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
        //��� �������� ��������� �� �������������� ���
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
    //���� � �������� ����� ������������ ���������, ������� ����������� �����.
    StopMap stop_names_;

    std::deque<Bus> buses_;
    //���� � �������� ����� ������������ ���������, ������� ����������� �����.
    BusMap bus_names_;

    DistanceMap distance_beetween_stops_;
};

}//��������� ������������ ��� transport_catalogue
