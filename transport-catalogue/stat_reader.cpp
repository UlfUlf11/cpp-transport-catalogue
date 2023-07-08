//чтение запросов на вывод и сам вывод;

#include "stat_reader.h"

namespace transport_catalogue {
    namespace output {

//пользователь запрашивает номер автобуса
void BusQuery(TransportCatalogue& catalogue, std::string_view bus_name) {
    //отсекаем слово Bus и пробел
    size_t bus_word = 4;
    bus_name = bus_name.substr(bus_word);

    Bus* bus = catalogue.GetBus(bus_name);
    if (bus != nullptr) {
        std::cout << "Bus " << bus->name << ": "
            << bus->stops.size() << " stops on route, "
            << catalogue.GetUniqueStopsForBus(bus) << " unique stops, "
            << catalogue.GetDistanceForBus(bus) << " route length, "
            << std::setprecision(6) //устанавливаем точность вывода координат
            << catalogue.GetDistanceForBus(bus) / catalogue.GetLength(bus) << " curvature" << std::endl;
    }
    else {
        std::cout << "Bus " << bus_name << ": not found" << std::endl;
    }
}

//пользователь хочет узнать, какие автобусы едут через остановку
void StopQuery(TransportCatalogue& catalogue, std::string_view stop_name) {
    //отсекаем слово Stop и пробел
    size_t stop_word = 5;
    stop_name = stop_name.substr(stop_word);

    std::unordered_set<const Bus*> unique_buses;
    std::vector <std::string> bus_names;

    Stop* stop = catalogue.GetStop(stop_name);

    if (stop != NULL) {
        unique_buses = catalogue.GetUniqueBusesForStop(stop);

        if (unique_buses.size() == 0) {
            std::cout << "Stop " << stop_name << ": no buses" << std::endl;
        }
        else {
            std::cout << "Stop " << stop_name << ": buses ";

            for (const Bus* bus : unique_buses) {
                bus_names.push_back(bus->name);
            }

            std::sort(bus_names.begin(), bus_names.end());

            for (std::string_view bus_name : bus_names) {
                std::cout << bus_name;
                std::cout << " ";
            }
            std::cout << std::endl;
        }
    }
    else {
        std::cout << "Stop " << stop_name << ": not found" << std::endl;
    }
}


void ParseQuery(TransportCatalogue& catalogue, std::string_view str) {
    if (str.substr(0, 3) == "Bus") {
        BusQuery(catalogue, str);
    }
    if (str.substr(0, 4) == "Stop") {
        StopQuery(catalogue, str);
    }
}


void Output(TransportCatalogue& catalogue) {
    std::string str_output_count;
    std::getline(std::cin, str_output_count);

    std::string str;
    std::vector<std::string> query;
    auto requests_count = stoi(str_output_count);

    for (int i = 0; i < requests_count; ++i) {
        std::getline(std::cin, str);
        query.push_back(str);
    }

    for (auto& request : query) {
        ParseQuery(catalogue, request);
    }
}

    }//завершаем пространство имён output
}//завершаем пространство имён transport_catalogue
