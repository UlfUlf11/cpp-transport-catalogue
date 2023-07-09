//чтение запросов на заполнение базы;

#include "input_reader.h"
#include "stat_reader.h"

namespace transport_catalogue {
    namespace input {


//из запроса пользователя создается остановка
Stop ParseStop(std::string str) {
    
    //двоеточие - признак окончания названия остановки
    size_t colon_pos = str.find(':');
    //запятая в запросе отделяет широту от долготы
    size_t comma_pos = str.find(',');
    //5 - это длина слова Stop и пробела после него в запросе
    size_t stop_word = 5;
    //от двоеточия, до начала ввода широты
    size_t space = 2;

    Stop stop;

    //отсекаем слово стоп и все после двоеточия вслючительно
    stop.name = str.substr(stop_word, colon_pos - stop_word);

    //stod извлекает число с плавающей точкой из строки str
    stop.coordinates.lat = stod(str.substr(colon_pos + space,
        comma_pos - colon_pos - space));

    stop.coordinates.lng = stod(str.substr(comma_pos + space));
    
    return stop;
}

//из запроса пользователя создается автобус
Bus ParseBus(TransportCatalogue& catalogue, std::string str) {
    //двоеточие - признак окончания названия автобуса
    size_t colon_pos = str.find(':');
    //4 - это длина слова Bus и пробела после него в запросе
    size_t bus_word = 4;
    //от двоеточия, до начала ввода маршрута
    size_t space = 2;

    Bus bus;

    //отсекаем слово bus и все после двоеточия вслючительно
    bus.name = str.substr(bus_word, colon_pos - bus_word);

    //отсекаем двоеточие и пробел после него
    str = str.substr(colon_pos + space);

    //ищем знак-разделитель для кольцевых маршрутов
    size_t greater_pos = str.find('>');

    if (greater_pos == std::string_view::npos) {
        //если не нашли >, ищем знак-разделитель для обратных маршрутов
        size_t hyphen_pos = str.find('-');

        while (hyphen_pos != std::string_view::npos) {
            bus.stops.push_back(catalogue.GetStop(str.substr(0, hyphen_pos - 1)));

            str = str.substr(hyphen_pos + space);
            hyphen_pos = str.find('-');
        }

        bus.stops.push_back(catalogue.GetStop(str.substr(0, hyphen_pos - 1)));
        size_t size = bus.stops.size() - 1;

        for (size_t i = size; i > 0; i--) {
            bus.stops.push_back(bus.stops[i - 1]);
        }

    }
    else {
        while (greater_pos != std::string_view::npos) {
            bus.stops.push_back(catalogue.GetStop(str.substr(0, greater_pos - 1)));

            str = str.substr(greater_pos + space);
            greater_pos = str.find('>');
        }

        bus.stops.push_back(catalogue.GetStop(str.substr(0, greater_pos - 1)));
    }
    return bus;
}

void ParseDistance(TransportCatalogue& catalogue, std::string str) {
    //двоеточие - признак окончания названия остановки
    size_t colon_pos = str.find(':');
    //5 - это длина слова Stop и пробела после него в запросе
    size_t stop_word = 5;
    //от двоеточия, до начала ввода широты
    size_t space = 2;

    //отсекаем слово стоп и все после двоеточия вслючительно
    std::string name = str.substr(stop_word, colon_pos - stop_word);
    //отсекаем от запроса широту, запятую после нее и пробел
    str = str.substr(str.find(',') + 1);
    //отсекаем от запроса долготу, запятую после нее и пробел
    //начало строки теперь = начало ввода расстояний между остановками
    str = str.substr(str.find(',') + space);

    //пока есть запятые => вводятся расстояния
    while (str.find(',') != std::string::npos) {

        //ищем символ задания расстояния
        int distance = stoi(str.substr(0, str.find('m')));
        //отсекаем букву м, слово to и 2 пробела (итого 5 символов)
        std::string distance_to = str.substr(str.find('m') + 5);
        //название станции, до которой вводится расстояние
        distance_to = distance_to.substr(0, distance_to.find(','));

        auto point_a = catalogue.GetStop(name);
        auto point_b = catalogue.GetStop(distance_to);

        catalogue.AddDistance(point_a, point_b, distance);

        str = str.substr(str.find(',') + space);

    }
    //обрабатываем расстояние до последней станции в запросе
    std::string last_station_name = str.substr(str.find('m') + stop_word);
    int distance = stoi(str.substr(0, str.find('m')));

    auto point_a = catalogue.GetStop(name);
    auto point_b = catalogue.GetStop(last_station_name);

    catalogue.AddDistance(point_a, point_b, distance);
}


//Запросы к базе данных подаются в cin после запросов на создание базы. В 
//первой строке вводится количество запросов, затем — по одному в строке — 
//вводятся сами запросы.

//Обратите внимание, что в маршруте может фигурировать остановка, объявленная 
//после этого маршрута. Рекомендуется сохранить запросы, и вначале обработать 
//все запросы остановок, а затем, все запросы маршрутов.
void Input(std::istream& in, TransportCatalogue& catalogue) {

    //кол-во запросов строкой
    std::string str_input_count;
    //считываем кол-во запросов
    std::getline(in, str_input_count);

    if (str_input_count != "") {
        //строка будущего запроса
        std::string input_query;
        
        std::vector<std::string> buses;
        std::vector<std::string> stops;
        //переводим кол-во запросов из строки в int
        int requests_count = stoi(str_input_count);
        //кол-во символов для запроса "Bus"
        int bus_word = 3;

        for (int i = 0; i < requests_count; ++i) {
            //считываем запрос
            std::getline(in, input_query);

            //обрабатываем запрос
            if (input_query != "") {
                auto space_pos = input_query.find_first_not_of(' ');
                input_query = input_query.substr(space_pos);

                //вначале нужно обработать все запросы остановок
                if (input_query.substr(0, bus_word) != "Bus") {
                    stops.push_back(input_query);
                }
                else {
                    buses.push_back(input_query);
                }
            }
        }

        for (auto stop : stops) {
            catalogue.AddStop(ParseStop(stop));
        }

        for (auto stop : stops) {
            ParseDistance(catalogue, stop);
        }

        for (auto& bus : buses) {
            catalogue.AddBus(ParseBus(catalogue, bus));
        }
    }
}

    }//завершаем пространство имён input
}//завершаем пространство имён transport_catalogue
