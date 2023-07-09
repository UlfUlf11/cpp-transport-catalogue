#pragma once

//чтение запросов на вывод и сам вывод;

#include "transport_catalogue.h"

namespace transport_catalogue {
	namespace output {

		void BusQuery(std::ostream& out,TransportCatalogue& catalogue, std::string_view bus_name);

		void StopQuery(std::ostream& out, TransportCatalogue& catalogue, std::string_view stop_name);

		void ParseQuery(std::ostream& out, TransportCatalogue& catalogue, std::string_view str);

		void Output(std::istream& in, std::ostream& out, TransportCatalogue& catalogue);

	}//завершаем пространство имён output
}//завершаем пространство имён transport_catalogue
