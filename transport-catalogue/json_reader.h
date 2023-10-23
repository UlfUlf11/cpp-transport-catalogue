#pragma once

#include "domain.h"
#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "serialization.h"

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
    std::string from;
    std::string to;
};

//функции заполняющие транспортный справочник

Stop ParseStop(Node& node);

Bus ParseBus(Node& node, TransportCatalogue& catalogue);

std::vector<std::tuple<Stop*, Stop*, int>> ParseDistance(
        Node& node, TransportCatalogue& catalogue);

void ParseBaseRequest(const Node& root, TransportCatalogue& catalogue);

svg::Color ReturnPalette(Array& arr_color);

//функция считывает настройки вывода карты
void ParseNodeRender(
    const Node& node, map_renderer::RenderSettings& rend_set);

//функция заполняет вектор <StatRequest> запросами на вывод
void ParseStatRequest(
    const Node& root, std::vector<StatRequest>& stat_request);

void ParseNodeRouting(const Node& node, TransportCatalogue& catalogue);

void ParseNodeSerialization(const Node& node, serialization::SerializationSettings& serialization_set);

void ParseNodeProcessRequest(std::vector<StatRequest>& stat_request,
                             serialization::SerializationSettings& serialization_settings);

//разделяем запросы на обращения к каталогу, ввод настроек отображения карты и
//вывод
void ParseNode(const Node& root, TransportCatalogue& catalogue,
               std::vector<StatRequest>& stat_request,
               map_renderer::RenderSettings& render_settings);

void MakeCatalogue(TransportCatalogue& catalogue,
                   map_renderer::RenderSettings& render_settings,
                   serialization::SerializationSettings& serialization_settings);

//функция ввода всего
void InputMakeBase(std::istream& input, TransportCatalogue& catalogue,
                   map_renderer::RenderSettings& render_settings,
                   serialization::SerializationSettings& serialization_settings);


void InputProcessRequest(std::istream& input, std::vector<StatRequest>& stat_request, serialization::SerializationSettings& serialization_settings);


} // end namespace input

namespace output
{
using namespace transport_catalogue::detail::json;
using namespace transport_catalogue::detail;

//вывод в формате json
void Output(TransportCatalogue& catalogue,
            std::vector<transport_catalogue::input::StatRequest>& stat_requests,
            request_handler::RequestHandler request_handler,
            graph::TransportRouter& router);

Node ExecuteMakeNodeStop(
    int id_request, transport_catalogue::detail::StopStat query_result);

Node ExecuteMakeNodeBus(
    int id_request, transport_catalogue::detail::BusStat query_result);

Node ReturnMapAsJsonNode(int id_request,
                         request_handler::RequestHandler request_handler,
                         TransportCatalogue& catalogue);

Node ExecuteMakeNodeRouter(transport_catalogue::input::StatRequest& request,
                           TransportCatalogue& catalogue, graph::TransportRouter& router);

} //завершаем пространство имён output

} // end namespace transport_catalogue
