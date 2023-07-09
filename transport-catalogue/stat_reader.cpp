//чтение запросов на вывод и сам вывод;

#include "stat_reader.h"

namespace transport_catalogue {
    namespace output {

//пользователь запрашивает номер автобуса
void BusQuery(std::ostream& out, TransportCatalogue& catalogue, std::string_view bus_name) {
    //отсекаем слово Bus и пробел
    size_t bus_word = 4;
    bus_name = bus_name.substr(bus_word);

    Bus* bus = catalogue.GetBus(bus_name);
    if (bus != nullptr) {
        out << "Bus " << bus->name << ": "
            << bus->stops.size() << " stops on route, "
            << catalogue.GetUniqueStopsForBus(bus).size() << " unique stops, "
            << catalogue.GetDistanceForBus(bus) << " route length, "
            << std::setprecision(6) //устанавливаем точность вывода координат
            << catalogue.GetDistanceForBus(bus) / catalogue.GetLength(bus) << " curvature" << std::endl;
    }
    else {
        out << "Bus " << bus_name << ": not found" << std::endl;
    }
}

//пользователь хочет узнать, какие автобусы едут через остановку
void StopQuery(std::ostream& out, TransportCatalogue& catalogue, std::string_view stop_name) {
    //отсекаем слово Stop и пробел
    size_t stop_word = 5;
    stop_name = stop_name.substr(stop_word);

    std::set<std::string> unique_buses;

    Stop* stop = catalogue.GetStop(stop_name);

    if (stop != NULL) {
        unique_buses = catalogue.GetUniqueBusesForStop(stop);

        if (unique_buses.size() == 0) {
            out << "Stop " << stop_name << ": no buses" << std::endl;
        }
        else {
            out << "Stop " << stop_name << ": buses ";

            for (const auto bus : unique_buses) {
                out << bus;
                out << " ";
            }

            out << std::endl;
        }
    }
    else {
        out << "Stop " << stop_name << ": not found" << std::endl;
    }
}


void ParseQuery(std::ostream& out, TransportCatalogue& catalogue, std::string_view str) {
    if (str.substr(0, 3) == "Bus") {
        BusQuery(out, catalogue, str);
    }
    if (str.substr(0, 4) == "Stop") {
        StopQuery(out, catalogue, str);
    }
}


void Output(std::istream& in, std::ostream& out, TransportCatalogue& catalogue) {
    std::string str_output_count;
    std::getline(in, str_output_count);

    std::string request;
    auto requests_count = stoi(str_output_count);

    for (int i = 0; i < requests_count; ++i) {
        std::getline(in, request);
        ParseQuery(out, catalogue, request);
    }
}

    }//завершаем пространство имён output
}//завершаем пространство имён transport_catalogue
