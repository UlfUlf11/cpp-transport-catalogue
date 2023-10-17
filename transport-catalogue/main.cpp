#include "domain.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_router.h"

using namespace std;

using namespace transport_catalogue;

using namespace transport_catalogue::detail;

using namespace transport_catalogue::detail::json;

int main()
{
  // stat_requests — массив с запросами к транспортному справочнику(заполняются
  // через Input)
  std::vector<transport_catalogue::input::StatRequest> stat_request;

  //настройки вывода изображения карты (заполняются через Input)
  map_renderer::RenderSettings render_settings;

  TransportCatalogue catalogue;

  /*
  Данные поступают из stdin в формате JSON-объекта. Его верхнеуровневая
  структура:
  {
    "base_requests": [ ... ],
    "render_settings": [ ... ],
    "routing_settings": [ ... ],
    "stat_requests": [ ... ]
  }
  */

  input::Input(std::cin, catalogue, stat_request, render_settings);

  //создаём визуализатор карты и с его помощью рисуем карту
  map_renderer::MapRenderer map_render(render_settings);

  //
  request_handler::RequestHandler request_handler(catalogue, map_render);

  // request_handler.RenderMap(std::cout);

  graph::TransportRouter router(catalogue); //создал маршутизатор

  //выводим все запросы
  output::Output(catalogue, stat_request, request_handler, router);
}