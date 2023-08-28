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

    //в каждом кортеже хранятся указатели на пару остановок и дистанция между ними
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
            //в каждом кортеже хранятся указатели на пару остановок и дистанция между ними
            std::vector<std::tuple<Stop*, Stop*, int>>  stops_and_distance = ParseDistance(stop, catalogue);

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


void ParceStatRequest(const Node& node, std::vector<StatRequest>& stat_request)
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

                if (req.type != "Map")
                {
                    req.name = req_map.at("name").AsString();
                }
                else
                {
                    req.name = "";
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


//считываем настройки вывода карты
void parce_node_render(const Node& node, map_renderer::RenderSettings& rend_set)
{
    //данные должны поступить в виде словаря
    Dict rend_map;

    Array bus_lab_offset;
    Array stop_lab_offset;
    Array arr_color;
    Array arr_palette;
    uint8_t red_;
    uint8_t green_;
    uint8_t blue_;
    double opacity_;

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
            rend_set.bus_label_offset_ = std::make_pair(bus_lab_offset[0].AsDouble(),
                                         bus_lab_offset[1].AsDouble());
        }

        rend_set.stop_label_font_size_ = rend_map.at("stop_label_font_size").AsInt();

        if (rend_map.at("stop_label_offset").IsArray())
        {
            stop_lab_offset = rend_map.at("stop_label_offset").AsArray();
            rend_set.stop_label_offset_ = std::make_pair(stop_lab_offset[0].AsDouble(),
                                          stop_lab_offset[1].AsDouble());
        }

        if (rend_map.at("underlayer_color").IsString())
        {
            rend_set.underlayer_color_ = svg::Color(rend_map.at("underlayer_color").AsString());
        }
        else if (rend_map.at("underlayer_color").IsArray())
        {
            arr_color = rend_map.at("underlayer_color").AsArray();
            red_ = arr_color[0].AsInt();
            green_ = arr_color[1].AsInt();
            blue_ = arr_color[2].AsInt();

            if (arr_color.size() == 4)
            {
                opacity_ = arr_color[3].AsDouble();
                rend_set.underlayer_color_ = svg::Color(svg::Rgba(red_,
                                                        green_,
                                                        blue_,
                                                        opacity_));
            }
            else if (arr_color.size() == 3)
            {
                rend_set.underlayer_color_ = svg::Color(svg::Rgb(red_,
                                                        green_,
                                                        blue_));
            }

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
                    red_ = arr_color[0].AsInt();
                    green_ = arr_color[1].AsInt();
                    blue_ = arr_color[2].AsInt();

                    if (arr_color.size() == 4)
                    {
                        opacity_ = arr_color[3].AsDouble();
                        rend_set.color_palette_.push_back(svg::Color(svg::Rgba(red_,
                                                          green_,
                                                          blue_,
                                                          opacity_)));
                    }
                    else if (arr_color.size() == 3)
                    {
                        rend_set.color_palette_.push_back(svg::Color(svg::Rgb(red_,
                                                          green_,
                                                          blue_)));
                    }
                }
            }
        }


    }
    else
    {
        std::cout << "render_settings is not map";
    }
}


//разделяем запросы на обращения к каталогу, ввод настроек отображения карты и вывод
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

        parce_node_render(dict.at("render_settings"), render_settings);

        ParceStatRequest(dict.at("stat_requests"), stat_request);

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

}//end namespace input

namespace output
{

using namespace transport_catalogue::detail::json;

void Output(TransportCatalogue& catalogue, std::vector<transport_catalogue::input::StatRequest>& stat_requests, request_handler::RequestHandler request_handler)
{
    std::vector<Node> result_request;
    //массив json на вывод
    Document doc_out;

    for (transport_catalogue::input::StatRequest req : stat_requests)
    {

        if (req.type == "Stop")
        {
            result_request.push_back(ReturnStopAsJsonNode(req.id, request_handler.StopQuery(catalogue, req.name)));

        }
        else if (req.type == "Bus")
        {
            result_request.push_back(ReturnBusAsJsonNode(req.id, request_handler.GetBusStat(catalogue, req.name)));
        }
        else if (req.type == "Map")
        {
            result_request.push_back(ReturnMapAsJsonNode(req.id, request_handler));
        }
    }
    doc_out = Document{ Node{result_request} };

    Print(doc_out, std::cout);
}

Node ReturnStopAsJsonNode(int id_request, transport_catalogue::detail::StopStat stop_info)
{
    Dict result;
    Array buses;
    std::string str_not_found = "not found";

    if (stop_info.not_found)
    {
        result.emplace("request_id", Node{ id_request });
        result.emplace("error_message", Node{ str_not_found });

    }
    else
    {
        result.emplace("request_id", Node{ id_request });

        for (std::string bus_name : stop_info.buses_name)
        {
            buses.push_back(Node{ bus_name });
        }

        result.emplace("buses", Node{ buses });
    }

    return Node{ result };
}

Node ReturnBusAsJsonNode(int id_request, transport_catalogue::detail::BusStat bus_info)
{
    Dict result;
    std::string str_not_found = "not found";

    if (bus_info.not_found)
    {
        result.emplace("request_id", Node{ id_request });
        result.emplace("error_message", Node{ str_not_found });

    }
    else
    {
        result.emplace("request_id", Node{ id_request });
        result.emplace("curvature", Node{ bus_info.curvature });
        result.emplace("route_length", Node{ bus_info.route_length });
        result.emplace("stop_count", Node{ bus_info.stops_on_route });
        result.emplace("unique_stop_count", Node{ bus_info.unique_stops });

    }

    return Node{ result };
}

Node ReturnMapAsJsonNode(int id_request, request_handler::RequestHandler request_handler)
{
    Dict result;
    std::ostringstream map_stream;
    std::string map_str;

    request_handler.RenderMap(map_stream);

    map_str = map_stream.str();

    result.emplace("request_id", Node(id_request));
    result.emplace("map", Node(map_str));

    return Node(result);
}

}//завершаем пространство имён output
}//end namespace transport_catalogue
