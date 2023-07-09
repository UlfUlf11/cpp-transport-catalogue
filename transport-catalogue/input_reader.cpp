//������ �������� �� ���������� ����;

#include "input_reader.h"
#include "stat_reader.h"

namespace transport_catalogue {
    namespace input {


//�� ������� ������������ ��������� ���������
Stop ParseStop(std::string str) {
    
    //��������� - ������� ��������� �������� ���������
    size_t colon_pos = str.find(':');
    //������� � ������� �������� ������ �� �������
    size_t comma_pos = str.find(',');
    //5 - ��� ����� ����� Stop � ������� ����� ���� � �������
    size_t stop_word = 5;
    //�� ���������, �� ������ ����� ������
    size_t space = 2;

    Stop stop;

    //�������� ����� ���� � ��� ����� ��������� ������������
    stop.name = str.substr(stop_word, colon_pos - stop_word);

    //stod ��������� ����� � ��������� ������ �� ������ str
    stop.coordinates.lat = stod(str.substr(colon_pos + space,
        comma_pos - colon_pos - space));

    stop.coordinates.lng = stod(str.substr(comma_pos + space));
    
    return stop;
}

//�� ������� ������������ ��������� �������
Bus ParseBus(TransportCatalogue& catalogue, std::string str) {
    //��������� - ������� ��������� �������� ��������
    size_t colon_pos = str.find(':');
    //4 - ��� ����� ����� Bus � ������� ����� ���� � �������
    size_t bus_word = 4;
    //�� ���������, �� ������ ����� ��������
    size_t space = 2;

    Bus bus;

    //�������� ����� bus � ��� ����� ��������� ������������
    bus.name = str.substr(bus_word, colon_pos - bus_word);

    //�������� ��������� � ������ ����� ����
    str = str.substr(colon_pos + space);

    //���� ����-����������� ��� ��������� ���������
    size_t greater_pos = str.find('>');

    if (greater_pos == std::string_view::npos) {
        //���� �� ����� >, ���� ����-����������� ��� �������� ���������
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
    //��������� - ������� ��������� �������� ���������
    size_t colon_pos = str.find(':');
    //5 - ��� ����� ����� Stop � ������� ����� ���� � �������
    size_t stop_word = 5;
    //�� ���������, �� ������ ����� ������
    size_t space = 2;

    //�������� ����� ���� � ��� ����� ��������� ������������
    std::string name = str.substr(stop_word, colon_pos - stop_word);
    //�������� �� ������� ������, ������� ����� ��� � ������
    str = str.substr(str.find(',') + 1);
    //�������� �� ������� �������, ������� ����� ��� � ������
    //������ ������ ������ = ������ ����� ���������� ����� �����������
    str = str.substr(str.find(',') + space);

    //���� ���� ������� => �������� ����������
    while (str.find(',') != std::string::npos) {

        //���� ������ ������� ����������
        int distance = stoi(str.substr(0, str.find('m')));
        //�������� ����� �, ����� to � 2 ������� (����� 5 ��������)
        std::string distance_to = str.substr(str.find('m') + 5);
        //�������� �������, �� ������� �������� ����������
        distance_to = distance_to.substr(0, distance_to.find(','));

        auto point_a = catalogue.GetStop(name);
        auto point_b = catalogue.GetStop(distance_to);

        catalogue.AddDistance(point_a, point_b, distance);

        str = str.substr(str.find(',') + space);

    }
    //������������ ���������� �� ��������� ������� � �������
    std::string last_station_name = str.substr(str.find('m') + stop_word);
    int distance = stoi(str.substr(0, str.find('m')));

    auto point_a = catalogue.GetStop(name);
    auto point_b = catalogue.GetStop(last_station_name);

    catalogue.AddDistance(point_a, point_b, distance);
}


//������� � ���� ������ �������� � cin ����� �������� �� �������� ����. � 
//������ ������ �������� ���������� ��������, ����� � �� ������ � ������ � 
//�������� ���� �������.

//�������� ��������, ��� � �������� ����� ������������ ���������, ����������� 
//����� ����� ��������. ������������� ��������� �������, � ������� ���������� 
//��� ������� ���������, � �����, ��� ������� ���������.
void Input(std::istream& in, TransportCatalogue& catalogue) {

    //���-�� �������� �������
    std::string str_input_count;
    //��������� ���-�� ��������
    std::getline(in, str_input_count);

    if (str_input_count != "") {
        //������ �������� �������
        std::string input_query;
        
        std::vector<std::string> buses;
        std::vector<std::string> stops;
        //��������� ���-�� �������� �� ������ � int
        int requests_count = stoi(str_input_count);
        //���-�� �������� ��� ������� "Bus"
        int bus_word = 3;

        for (int i = 0; i < requests_count; ++i) {
            //��������� ������
            std::getline(in, input_query);

            //������������ ������
            if (input_query != "") {
                auto space_pos = input_query.find_first_not_of(' ');
                input_query = input_query.substr(space_pos);

                //������� ����� ���������� ��� ������� ���������
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

    }//��������� ������������ ��� input
}//��������� ������������ ��� transport_catalogue
