#include "json_reader.h"

namespace transport_catalogue

{
    namespace input
    {
        using namespace transport_catalogue::detail::json;

        //из запроса пользователя создается остановка
        Stop ParseStop(Node& node)
        {
            Stop stop;

            Dict stop_node;

            if (node.IsMap())
            {
                stop_node = node.AsMap();
                stop.name = stop_node.at("name").AsString();
                stop.coordinates.latitude = stop_node.at("latitude").AsDouble();
                stop.coordinates.longitude = stop_node.at("longitude").AsDouble();
            }

            return stop;
        }

        //из запроса пользователя создается автобус
        Bus ParseBus(Node& node, TransportCatalogue& catalogue)
        {
            Bus bus;

            Dict bus_node;

            Array bus_stops;

            if (node.IsMap())
            {
                bus_node = node.AsMap();
                bus.name = bus_node.at("name").AsString();
                bus.is_roundtrip = bus_node.at("is_roundtrip").AsBool();

                try
                {
                    bus_stops = bus_node.at("stops").AsArray();

                    for (Node stop : bus_stops)
                    {
                        bus.stops.push_back(catalogue.GetStop(stop.AsString()));
                    }

                    if (!bus.is_roundtrip)
                    {
                        size_t size = bus.stops.size() - 1;

                        for (size_t i = size; i > 0; i--)
                        {
                            bus.stops.push_back(bus.stops[i - 1]);
                        }
                    }
                }

                catch (...)
                {
                    std::cout << "Error: base_requests: bus: stops is empty" << std::endl;
                }
            }

            return bus;
        }

        //добавляется дистанция между остановками
        std::vector<std::tuple<Stop*, Stop*, int>> ParseDistance(Node& node, TransportCatalogue& catalogue)
        {
            //в каждом кортеже хранятся указатели на пару остановок и дистанция между
            //ними
            std::vector<std::tuple<Stop*, Stop*, int>> distances;

            std::tuple<Stop*, Stop*, int> stops_and_distance;

            Dict stop_node;
            Dict stop_road_map;

            std::string begin_name;
            std::string last_name;

            int distance;

            if (node.IsMap())
            {
                stop_node = node.AsMap();
                begin_name = stop_node.at("name").AsString();

                try
                {
                    stop_road_map = stop_node.at("road_distances").AsMap();

                    for (auto [key, value] : stop_road_map)
                    {
                        last_name = key;

                        distance = value.AsInt();

                        stops_and_distance = std::make_tuple(catalogue.GetStop(begin_name), catalogue.GetStop(last_name), distance);

                        distances.push_back(stops_and_distance);
                    }
                }

                catch (...)
                {
                    std::cout << "Error: Road invalide" << std::endl;
                }
            }

            return distances;
        }

        void ParseBaseRequest(const Node& root, TransportCatalogue& catalogue)
        {
            Array base_requests;

            Dict req_map;

            Node req_node;

            std::vector<Node> buses;
            std::vector<Node> stops;

            if (root.IsArray())
            {
                base_requests = root.AsArray();

                for (Node& node : base_requests)
                {
                    if (node.IsMap())
                    {
                        req_map = node.AsMap();

                        try
                        {
                            req_node = req_map.at("type");

                            if (req_node.IsString())
                            {
                                if (req_node.AsString() == "Bus")
                                {
                                    buses.push_back(req_map);
                                }

                                else if (req_node.AsString() == "Stop")
                                {
                                    stops.push_back(req_map);
                                }

                                else
                                {
                                    std::cout << "Error: base_requests have bad type";
                                }
                            }
                        }

                        catch (...)
                        {
                            std::cout << "Error: base_requests not have type value";
                        }
                    }
                }

                for (auto stop : stops)
                {
                    catalogue.AddStop(ParseStop(stop));
                }

                for (auto stop : stops)
                {
                    //в каждом кортеже хранятся указатели на пару остановок и дистанция
                    //между ними
                    std::vector<std::tuple<Stop*, Stop*, int>> stops_and_distance = ParseDistance(stop, catalogue);

                    for (auto stops_pair : stops_and_distance)
                    {
                        Stop* stop_a = std::get<0>(stops_pair);
                        Stop* stop_b = std::get<1>(stops_pair);

                        int distance = std::get<2>(stops_pair);

                        catalogue.AddDistance(stop_a, stop_b, distance);
                    }
                }

                for (auto bus : buses)
                {
                    catalogue.AddBus(ParseBus(bus, catalogue));
                }
            }

            else
            {
                std::cout << "base_requests is not array";
            }
        }

        void ParseStatRequest(const Node& node, std::vector<StatRequest>& stat_request)
        {
            Array stat_requests;
            Dict req_map;
            StatRequest req;

            if (node.IsArray())
            {
                stat_requests = node.AsArray();

                for (Node req_node : stat_requests)
                {
                    if (req_node.IsMap())
                    {
                        req_map = req_node.AsMap();
                        req.id = req_map.at("id").AsInt();
                        req.type = req_map.at("type").AsString();

                        if ((req.type == "Bus") || (req.type == "Stop"))

                        {
                            req.name = req_map.at("name").AsString();
                            req.from = "";
                            req.to = "";
                        }

                        else {
                            req.name = "";
                            if (req.type == "Route") {
                                req.from = req_map.at("from").AsString();
                                req.to = req_map.at("to").AsString();
                            }
                            else {
                                req.from = "";
                                req.to = "";
                            }
                        }

                        stat_request.push_back(req);
                    }
                }
            }

            else
            {
                std::cout << "Error: base_requests is not array";
            }
        }

        svg::Color ReturnPalette(Array& arr_color)
        {
            uint8_t red_ = arr_color[0].AsInt();
            uint8_t green_ = arr_color[1].AsInt();
            uint8_t blue_ = arr_color[2].AsInt();
            double opacity_;

            if (arr_color.size() == 4)
            {
                opacity_ = arr_color[3].AsDouble();
                return svg::Color(svg::Rgba(red_, green_, blue_, opacity_));
            }

            else if (arr_color.size() == 3)
            {
                return svg::Color(svg::Rgb(red_, green_, blue_));
            }
            return std::monostate();
        }

        //считываем настройки вывода карты
        void ParseNodeRender(const Node& node, map_renderer::RenderSettings& rend_set)
        {
            //данные должны поступить в виде словаря
            Dict rend_map;

            Array bus_lab_offset;
            Array stop_lab_offset;
            Array arr_color;
            Array arr_palette;

            if (node.IsMap())
            {
                rend_map = node.AsMap();
                rend_set.width_ = rend_map.at("width").AsDouble();
                rend_set.height_ = rend_map.at("height").AsDouble();
                rend_set.padding_ = rend_map.at("padding").AsDouble();
                rend_set.line_width_ = rend_map.at("line_width").AsDouble();
                rend_set.stop_radius_ = rend_map.at("stop_radius").AsDouble();
                rend_set.bus_label_font_size_ = rend_map.at("bus_label_font_size").AsInt();

                if (rend_map.at("bus_label_offset").IsArray())
                {
                    bus_lab_offset = rend_map.at("bus_label_offset").AsArray();

                    rend_set.bus_label_offset_ = std::make_pair(bus_lab_offset[0].AsDouble(), bus_lab_offset[1].AsDouble());
                }

                rend_set.stop_label_font_size_
                    = rend_map.at("stop_label_font_size").AsInt();

                if (rend_map.at("stop_label_offset").IsArray())
                {
                    stop_lab_offset = rend_map.at("stop_label_offset").AsArray();

                    rend_set.stop_label_offset_
                        = std::make_pair(stop_lab_offset[0].AsDouble(),

                            stop_lab_offset[1].AsDouble());
                }

                if (rend_map.at("underlayer_color").IsString())
                {
                    rend_set.underlayer_color_
                        = svg::Color(rend_map.at("underlayer_color").AsString());
                }

                else if (rend_map.at("underlayer_color").IsArray())
                {
                    arr_color = rend_map.at("underlayer_color").AsArray();
                    rend_set.underlayer_color_ = ReturnPalette(arr_color);
                }

                rend_set.underlayer_width_ = rend_map.at("underlayer_width").AsDouble();

                if (rend_map.at("color_palette").IsArray())
                {
                    arr_palette = rend_map.at("color_palette").AsArray();

                    for (Node color_palette : arr_palette)
                    {
                        if (color_palette.IsString())
                        {
                            rend_set.color_palette_.push_back(svg::Color(color_palette.AsString()));
                        }

                        else if (color_palette.IsArray())
                        {
                            arr_color = color_palette.AsArray();
                            rend_set.color_palette_.push_back(ReturnPalette(arr_color));
                        }
                    }
                }
            }

            else
            {
                std::cout << "render_settings is not map";
            }
        }

        void ParseNodeRouting(const Node& node, TransportCatalogue& catalogue)
        {
            Dict route;

            RoutingSettings routing_settings;

            if (node.IsMap()) {
                route = node.AsMap();

                routing_settings.bus_wait_time = route.at("bus_wait_time").AsDouble();
                routing_settings.bus_velocity = route.at("bus_velocity").AsDouble();

                catalogue.AddRouteSettings(routing_settings);
            }
            else {
                std::cout << "routing settings is not map";
            }
        }

        //разделяем запросы на обращения к каталогу, ввод настроек отображения карты и
        //вывод
        void ParseNode(const Node& root, TransportCatalogue& catalogue, [[maybe_unused]] std::vector<StatRequest>& stat_request, map_renderer::RenderSettings& render_settings)

        {
            Dict dict;

            //запрос должен быть словарем
            if (root.IsMap())
            {
                //получаем словарь со всеми запросами
                dict = root.AsMap();

                //передаем массивы обращений к каталогу и запросов на вывод
                ParseBaseRequest(dict.at("base_requests"), catalogue);
                ParseNodeRender(dict.at("render_settings"), render_settings);
                ParseStatRequest(dict.at("stat_requests"), stat_request);
                ParseNodeRouting(dict.at("routing_settings"), catalogue);

            }

            else
            {
                std::cout << "Top level request is not map";
            }
        }

        void Input(std::istream& input, TransportCatalogue& catalogue, std::vector<StatRequest>& stat_request, map_renderer::RenderSettings& render_settings)
        {
            //данные на ввод и вывод
            Document doc;
            doc = Load(input);
            ParseNode(doc.GetRoot(), catalogue, stat_request, render_settings);
        }

    } // end namespace input

    namespace output {
        using namespace transport_catalogue::detail::json;

        void Output(TransportCatalogue& catalogue,
            std::vector<transport_catalogue::input::StatRequest>& stat_requests,
            request_handler::RequestHandler request_handler,
            graph::TransportRouter& router)
        {
            std::vector<Node> result_request;

            //массив json на вывод
            Document doc_out;

            for (transport_catalogue::input::StatRequest req : stat_requests) {
                if (req.type == "Stop") {
                    result_request.push_back(ExecuteMakeNodeStop(
                        req.id, request_handler.StopQuery(catalogue, req.name)));
                }

                else if (req.type == "Bus") {
                    result_request.push_back(ExecuteMakeNodeBus(
                        req.id, request_handler.GetBusStat(catalogue, req.name)));
                }

                else if (req.type == "Map") {
                    result_request.push_back(
                        ReturnMapAsJsonNode(req.id, request_handler, catalogue));
                }
                else if (req.type == "Route") {
                    result_request.push_back(ExecuteMakeNodeRouter(req, catalogue, router));
                }
            }

            doc_out = Document{ Node { result_request } };

            Print(doc_out, std::cout);
        }

        Node ExecuteMakeNodeStop(int id_request, transport_catalogue::detail::StopStat stop_info)
        {
            Node result;
            Array buses;
            Builder builder;

            std::string str_not_found = "not found";

            if (stop_info.not_found)
            {
                builder
                    .StartDict()
                    .Key("request_id")
                    .Value(id_request)
                    .Key("error_message")
                    .Value(str_not_found)
                    .EndDict();

                result = builder.Build();
            }

            else
            {
                builder
                    .StartDict()
                    .Key("request_id")
                    .Value(id_request)
                    .Key("buses")
                    .StartArray();

                for (std::string bus_name : stop_info.buses_name)
                {
                    builder.Value(bus_name);
                }

                builder.EndArray().EndDict();

                result = builder.Build();
            }

            return result;
        }

        Node ExecuteMakeNodeBus(int id_request, transport_catalogue::detail::BusStat bus_info)
        {
            Node result;
            Builder builder;

            std::string str_not_found = "not found";

            if (bus_info.not_found)
            {
                builder
                    .StartDict()
                    .Key("request_id")
                    .Value(id_request)
                    .Key("error_message")
                    .Value(str_not_found)
                    .EndDict();

                result = builder.Build();
            }

            else
            {
                builder
                    .StartDict()
                    .Key("request_id")
                    .Value(id_request)
                    .Key("curvature")
                    .Value(bus_info.curvature)
                    .Key("route_length")
                    .Value(bus_info.route_length)
                    .Key("stop_count")
                    .Value(bus_info.stops_on_route)
                    .Key("unique_stop_count")
                    .Value(bus_info.unique_stops)
                    .EndDict();

                result = builder.Build();
            }

            return result;
        }

        Node ReturnMapAsJsonNode(int id_request, request_handler::RequestHandler request_handler, TransportCatalogue& catalogue)
        {
            Node result;
            Builder builder;

            std::ostringstream map_stream;
            std::string map_str;

            request_handler.RenderMap(map_stream, catalogue);

            map_str = map_stream.str();

            builder
                .StartDict()
                .Key("request_id")
                .Value(id_request)
                .Key("map")
                .Value(map_str)
                .EndDict();

            result = builder.Build();

            return result;
        }

        Node ExecuteMakeNodeRouter(transport_catalogue::input::StatRequest& request, TransportCatalogue& catalogue, graph::TransportRouter& router)
        {
            //если есть остановки...
            if (catalogue.GetStop(request.from) && catalogue.GetStop(request.to)) {

                std::vector<std::variant<graph::RouteBus, graph::RouteWait>> final_route
                    = router.GetRoute(request.from, request.to);
                std::vector<json::Node> array;

                int request_id = request.id;
                double total_time = 0;
                std::string error_message = "not found";
                bool empty_request = false;

                for (auto el : final_route) {

                    if (std::holds_alternative<graph::RouteBus>(el)) {
                        graph::RouteBus act = std::get<graph::RouteBus>(el);
                        if (act.bus_name == "not found") {
                            json::Node final_route_description = json::Builder{}
                                .StartDict()
                                .Key("request_id")
                                .Value(request_id)
                                .Key("error_message")
                                .Value(error_message)
                                .EndDict()
                                .Build();

                            return final_route_description;
                            empty_request = true;
                            break;
                        }
                        else {
                            json::Node bus_route_description = json::Builder{}
                                .StartDict()
                                .Key("bus")
                                .Value(act.bus_name)
                                .Key("span_count")
                                .Value(act.span_count)
                                .Key("time")
                                .Value(act.time)
                                .Key("type")
                                .Value(act.type_activity)
                                .EndDict()
                                .Build();

                            total_time += act.time;
                            array.push_back(bus_route_description);
                        }
                    }

                    if (std::holds_alternative<graph::RouteWait>(el)) {
                        graph::RouteWait act = std::get<graph::RouteWait>(el);

                        json::Node bus_route_description = json::Builder{}
                            .StartDict()
                            .Key("stop_name")
                            .Value(act.stop_name_from)
                            .Key("time")
                            .Value(act.time)
                            .Key("type")
                            .Value(act.type_activity)
                            .EndDict()
                            .Build();

                        total_time += act.time;
                        array.push_back(bus_route_description);
                    }
                }

                if (!empty_request) {
                    json::Node final_route_description = json::Builder{}
                        .StartDict()
                        .Key("items")
                        .Value(array)
                        .Key("request_id")
                        .Value(request_id)
                        .Key("total_time")
                        .Value(total_time)
                        .EndDict()
                        .Build();

                    return final_route_description;
                }
            }

            int request_id = request.id;
            std::string error_message = "not found";
            json::Node final_route_description = json::Builder{}
                .StartDict()
                .Key("request_id")
                .Value(request_id)
                .Key("error_message")
                .Value(error_message)
                .EndDict()
                .Build();

            return final_route_description;
        }

    } //завершаем пространство имён output
} // end namespace transport_catalogue
