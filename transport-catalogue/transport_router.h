#pragma once

#include "iostream"
#include <algorithm>
#include <iostream>
#include <map>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
#include <variant>

#include "transport_catalogue.h"
#include "ranges.h"
#include "router.h"
#include "graph.h"

namespace graph
{

using namespace transport_catalogue;

static const uint16_t METERS_IN_KILOMETER = 1000;
static const uint16_t MINUTES_IN_HOUR = 60;

struct RouteBus
{
    std::string bus_name;
    double time;
    int span_count;
    std::string type_activity = "Bus";
};

struct RouteWait
{
    std::string stop_name_from;
    double time;
    std::string type_activity = "Wait";
};

class TransportRouter
{

public:
    TransportRouter(transport_catalogue::TransportCatalogue& tc);

    std::vector<std::variant<RouteBus, RouteWait>> GetRoute(std::string& stop_from, std::string& stop_to);

private:
    transport_catalogue::TransportCatalogue& tc;
    std::vector<Edge<double>*> added_edges_;
    DirectedWeightedGraph<double> graph_;
    //Имя и номер вершины
    std::unordered_map<std::string, size_t> stop_to_vertex_;
    //Маршутизатор, который будет искать оптимальный путь
    std::unique_ptr<graph::Router<double>> router_;

    bool CheсkStopExist(std::string& key);

    void FillingGraph();

    void AddStopsOneDirection(const std::deque<transport_catalogue::detail::Stop*>& stops, const std::string& bus_name);

    void AddStopsTwoDirections(const std::deque<transport_catalogue::detail::Stop*>& stops, const std::string& bus_name);

};
}
