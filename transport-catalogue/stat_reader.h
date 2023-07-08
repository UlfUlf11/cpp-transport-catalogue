#pragma once

//чтение запросов на вывод и сам вывод;

#include "transport_catalogue.h"

namespace transport_catalogue {
	namespace output {

		void BusQuery(TransportCatalogue& catalogue, std::string_view bus_name);

		void StopQuery(TransportCatalogue& catalogue, std::string_view stop_name);

		void ParseQuery(TransportCatalogue& catalogue, std::string_view str);

		void Output(TransportCatalogue& catalogue);

	}//завершаем пространство имён output
}//завершаем пространство имён transport_catalogue
