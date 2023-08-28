#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "domain.h"
#include "map_renderer.h"
#include "request_handler.h"

namespace transport_catalogue
{
namespace input
{

using namespace transport_catalogue::detail::json;
using namespace transport_catalogue::detail;

/*
Формат запроса:
{
  "id": 12345678,
  "type": "Bus",
  "name": "14"
}
*/
struct StatRequest
{
    int id;
    std::string type;
    std::string name;
};

//функции заполняющие транспортный справочник
Stop ParseStop(Node& node);
Bus ParseBus(Node& node, TransportCatalogue& catalogue);
std::vector<std::tuple<Stop*, Stop*, int>> ParseDistance(Node& node, TransportCatalogue& catalogue);
void ParseBaseRequest(const Node& root, TransportCatalogue& catalogue);

//функция считывает настройки вывода карты
void parse_node_render(const Node& node, map_renderer::RenderSettings& rend_set);

//функция заполняет вектор <StatRequest> запросами на вывод
void ParceStatRequest(const Node& root, std::vector<StatRequest>& stat_request);

//разделяем запросы на обращения к каталогу, ввод настроек отображения карты и вывод
void ParseNode(const Node& root, TransportCatalogue& catalogue, std::vector<StatRequest>& stat_request, map_renderer::RenderSettings& render_settings);

//функция ввода всего
void Input(std::istream& input, TransportCatalogue& catalogue, std::vector<StatRequest>& stat_request, map_renderer::RenderSettings& render_settings);

}//end namespace input

namespace output
{

using namespace transport_catalogue::detail::json;
using namespace transport_catalogue::detail;

Node ReturnStopAsJsonNode(int id_request, transport_catalogue::detail::StopStat query_result);
Node ReturnBusAsJsonNode(int id_request, transport_catalogue::detail::BusStat query_result);
Node ReturnMapAsJsonNode(int id_request, request_handler::RequestHandler request_handler);
    
//вывод в формате json
void Output(TransportCatalogue& catalogue, std::vector<transport_catalogue::input::StatRequest>& stat_requests, request_handler::RequestHandler request_handler);    

}//завершаем пространство имён output
}//end namespace transport_catalogue
