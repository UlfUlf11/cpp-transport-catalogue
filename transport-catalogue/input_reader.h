#pragma once

//������ �������� �� ���������� ����;
#include <algorithm>

#include "transport_catalogue.h"

namespace transport_catalogue {
	namespace input {

		Stop ParseStop(std::string_view str);

		Bus ParseBus(TransportCatalogue& catalogue, std::string_view str);

		void ParseDistance(TransportCatalogue& catalogue, std::string str);

		void Input(std::istream& in, TransportCatalogue& catalogue);

	}//��������� ������������ ��� input
}//��������� ������������ ��� transport_catalogue
