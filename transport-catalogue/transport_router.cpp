#include "transport_router.h"

namespace graph
{

TransportRouter::TransportRouter(transport_catalogue::TransportCatalogue& tc)
    : tc(tc)
{
    //Сделаем граф, передав конструктору число (количество остановок * 2)
    //Для каждой остановки 2 вершины: вход остановки и выход с остановки.
    graph_ = DirectedWeightedGraph<double>(2 * tc.GetStopsQuantity());

    FillingGraph();

    //конструируем из графа маршутизатор. Он будет искать оптимальный
    //маршрут.
    router_ = std::unique_ptr<graph::Router<double>>(
                  new graph::Router<double>(graph_));
}

void TransportRouter::FillingGraph()
{
    const std::deque<detail::Bus>& buses_ = tc.GetBuses();
    /*
    автобусы {
    остановки {
    остановки {}
        }
    }
    */

    for (const detail::Bus& bus : buses_)
    {

        const std::deque<Stop*>& stops = bus.stops;

        if (bus.is_roundtrip == true)
        {
            AddStopsOneDirection(stops, bus.name);
        }
        else
        {
            AddStopsTwoDirections(stops, bus.name);
        }
    }
}

std::vector<std::variant<RouteBus, RouteWait>> TransportRouter::GetRoute(
            std::string& stop_from, std::string& stop_to)
{
    std::vector<std::variant<RouteBus, RouteWait>> final_route;

    // id остановок
    size_t from;
    size_t to;

    if (!CheсkStopExist(stop_from) || !CheсkStopExist(stop_to))
    {
        RouteBus bus_route;
        bus_route.bus_name = "not found";
        final_route.push_back(bus_route);

        return final_route;
    }

    from = stop_to_vertex_.find(stop_from)->second;
    to = stop_to_vertex_.find(stop_to)->second;

    std::optional<typename graph::Router<double>::RouteInfo> route_info
        = router_->BuildRoute(from, to);

    double wait_time = tc.GetWaitTime();

    std::set<std::string> actual_buses;
    std::set<std::string> buses_for_route_first_stop;
    std::set<std::string> buses_for_route_second_stop;

    if (route_info.has_value())
    {
        const auto& route_info_value = route_info.value();

        for (auto it = route_info_value.edges.begin();
                it != route_info_value.edges.end(); ++it)
        {
            auto EdgId = *it;
            auto Edge = graph_.GetEdge(EdgId);
            if (Edge.stop_count == 0)
            {
                RouteWait route_wait;
                route_wait.time = wait_time;
                route_wait.stop_name_from = Edge.name;

                final_route.push_back(route_wait);

            }

            else
            {
                RouteBus bus_route;
                bus_route.bus_name = Edge.name;
                bus_route.time = Edge.weight;
                bus_route.span_count = Edge.stop_count;
                final_route.push_back(bus_route);
            }
        }
        return final_route;
    }
    else
    {
        RouteBus bus_route;
        bus_route.bus_name = "not found";
        final_route.push_back(bus_route);

        return final_route;
    }
}


bool TransportRouter::CheсkStopExist(std::string& key)
{
    auto it = stop_to_vertex_.find(key);
    if (it != stop_to_vertex_.end())
    {
        return true;
    }
    else
    {
        return false;
    }
}

void TransportRouter::AddStopsOneDirection(const std::deque<Stop*>& stops, const std::string& bus_name)
{
    /*
    Для каждой остановки 2 вершины: вход остановки и выход с остановки.
    Номера вершин: вход - номер остановки * 2, выход - номер остановки * 2
    + 1.
    */

    for (auto it = stops.begin(); std::next(it) != stops.end(); ++it)
    {
        double sum_time = 0; // время движения по маршруту

        // id вершин для ожидания на остановках
        size_t vertex1_wait_id;
        size_t vertex_next_wait_id;

        //берем остановку, от которой поедем...
        detail::Stop entrance = **it;
        std::string entrance_name = entrance.name;

        //проверяем, не добавлена ли она уже...
        if (CheсkStopExist(entrance_name))
        {
            auto indx = stop_to_vertex_.find(entrance_name);
            vertex1_wait_id = indx->second;
        }
        else
        {
            // Номер вершины "вход": номер остановки * 2
            vertex1_wait_id = stop_to_vertex_.size() * 2;
            //добавляем название остановки и id
            stop_to_vertex_.insert({ entrance_name, vertex1_wait_id });
        }

        detail::Stop next_station = **(std::next(it));
        std::string next_station_name = next_station.name;

        if (CheсkStopExist(next_station_name))
        {
            auto indx = stop_to_vertex_.find(next_station_name);
            vertex_next_wait_id = indx->second;
        }
        else
        {
            // Номер вершины "вход": номер остановки * 2
            vertex_next_wait_id = stop_to_vertex_.size() * 2;
            //добавляем название остановки и id
            stop_to_vertex_.insert(
            { next_station_name, vertex_next_wait_id });
        }

        // id вершины для поездки
        size_t vertex1_go_id = vertex1_wait_id + 1;

        const detail::Stop* stop1 = tc.GetStop(entrance_name);
        const detail::Stop* stop1_next = tc.GetStop(next_station_name);

        int distance = tc.GetDistanceForStop(
                           stop1, stop1_next); // расстояние между остановки
        double time
            = distance / (tc.GetVelocity() * METERS_IN_KILOMETER / MINUTES_IN_HOUR) + sum_time;

        //
        Edge<double>* edge_wait_go = new Edge<double> { vertex1_wait_id,
                vertex1_go_id, tc.GetWaitTime(), entrance_name, 0
                                                      };
        Edge<double>* edge_go_wait = new Edge<double> { vertex1_go_id,
                vertex_next_wait_id, time, bus_name, 1
                                                      };

        graph_.AddEdge(*edge_wait_go);
        added_edges_.push_back(edge_wait_go);

        graph_.AddEdge(*edge_go_wait);
        added_edges_.push_back(edge_go_wait);

        sum_time = time;

        //обрабатываем все последующие остановки во внутреннем цикле...
        for (auto it_inner = std::next(it);
                std::next(it_inner) != stops.end(); ++it_inner)
        {

            size_t vertex1_inner_next_wait_id;

            detail::Stop stop1_inner = **(it_inner);
            std::string stop1_inner_name = stop1_inner.name;

            detail::Stop stop1_inner_next = **std::next(it_inner);
            std::string stop1_inner_next_name = stop1_inner_next.name;

            //проверяем, не добавлена ли остановка уже...
            if (CheсkStopExist(stop1_inner_next_name))
            {
                auto indx = stop_to_vertex_.find(stop1_inner_next_name);
                vertex1_inner_next_wait_id = indx->second;
            }
            else
            {
                // Номер вершины "вход": номер остановки * 2
                vertex1_inner_next_wait_id = stop_to_vertex_.size() * 2;
                //добавляем название остановки и id
                stop_to_vertex_.insert(
                { stop1_inner_next_name, vertex1_inner_next_wait_id });
            }

            const detail::Stop* stop1_inner_ptr
                = tc.GetStop(stop1_inner_name);
            const detail::Stop* stop1_inner_next_ptr
                = tc.GetStop(stop1_inner_next_name);

            int distance_inner = tc.GetDistanceForStop(stop1_inner_ptr,
                                 stop1_inner_next_ptr); // расстояние между остановками

            double time_inner
                = distance_inner / (tc.GetVelocity() * 1000 / 60)
                  + sum_time; // суммирую время с тем, что уже было накоплено

            //кол-во проеханных остановок
            int span_count
                = std::distance(stops.begin(), std::next(it_inner))
                  - std::distance(stops.begin(), it);

            Edge<double>* edge_go_wait_inner
            = new Edge<double> { vertex1_go_id,
                                 vertex1_inner_next_wait_id, time_inner, bus_name,
                                 span_count
                               };

            graph_.AddEdge(*edge_go_wait_inner);
            added_edges_.push_back(edge_go_wait_inner);
            sum_time = time_inner; // обновляю суммарное время
        }
    }
}

void TransportRouter::AddStopsTwoDirections(const std::deque<Stop*>& stops, const std::string& bus_name)
{
    AddStopsOneDirection(stops, bus_name); // Заполняю в одном направлении

    std::deque<Stop*> stops_reverse(stops.rbegin(), stops.rend());

    AddStopsOneDirection(stops_reverse, bus_name); // Заполняю в обратном направлении
}

} //end namespace graph
