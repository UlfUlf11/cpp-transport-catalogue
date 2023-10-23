#include <fstream>
#include <iostream>
#include <string_view>

#include "domain.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "serialization.h"

using namespace std::literals;
using namespace std;
using namespace transport_catalogue;
using namespace transport_catalogue::detail;
using namespace serialization;

void PrintUsage(std::ostream& stream = std::cerr)
{
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    SerializationSettings serialization_settings;

    if (mode == "make_base"sv)
    {

        TransportCatalogue catalogue;

        //настройки вывода изображения карты (заполняются через Input)
        map_renderer::RenderSettings render_settings;

        /*
        Данные поступают из stdin в формате JSON-объекта. Его верхнеуровневая
        структура:
        {
          "serialization_settings": [ ... ],
          "routing_settings": [ ... ],
          "render_settings": [ ... ],
          "base_requests": [ ... ],
        }
        */

        input::InputMakeBase(std::cin, catalogue, render_settings, serialization_settings);

        ofstream out_file(serialization_settings.file_name, ios::binary);

        RoutingSettings routing_settings = catalogue.GetRouteSettings();

        CatalogueSerialization(catalogue, render_settings, routing_settings, out_file);

    }
    else if (mode == "process_requests"sv)
    {

        // stat_requests — массив с запросами к транспортному справочнику(заполняются
        // через InputForDeserialization)
        std::vector<transport_catalogue::input::StatRequest> stat_request;

        input::InputProcessRequest(std::cin, stat_request, serialization_settings);

        ifstream in_file(serialization_settings.file_name, ios::binary);

        //десериализованная база данных по транспортному каталогу
        Catalogue deserialized_catalogue = CatalogueDeserialization(in_file);

        RenderSettings render_settings = deserialized_catalogue.render_settings_;
        TransportCatalogue catalogue = deserialized_catalogue.transport_catalogue_;
        //т.к. в этом режиме каталог не заполнялся настройками routing_settings,
        //то дополнительно инициализирую эти поля
        catalogue.AddRouteSettings(deserialized_catalogue.routing_settings_);

        //создаём визуализатор карты и с его помощью рисуем карту
        map_renderer::MapRenderer map_render(render_settings);

        request_handler::RequestHandler request_handler(catalogue, map_render);
        
        graph::TransportRouter router(catalogue); //создал маршутизатор

        //выводим все запросы
        output::Output(catalogue, stat_request, request_handler, router);
    }
    else
    {
        PrintUsage();
        return 1;
    }
}
