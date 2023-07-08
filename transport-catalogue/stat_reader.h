#pragma once

//������ �������� �� ����� � ��� �����;

#include "transport_catalogue.h"

namespace transport_catalogue {
	namespace output {

		void BusQuery(TransportCatalogue& catalogue, std::string_view bus_name);

		void StopQuery(TransportCatalogue& catalogue, std::string_view stop_name);

		void ParseQuery(TransportCatalogue& catalogue, std::string_view str);

		void Output(TransportCatalogue& catalogue);

	}//��������� ������������ ��� output
}//��������� ������������ ��� transport_catalogue
