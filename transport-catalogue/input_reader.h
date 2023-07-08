#pragma once

//������ �������� �� ���������� ����;
#include <algorithm>

#include "transport_catalogue.h"

namespace transport_catalogue {
	namespace input {

		Stop ParseStop(std::string_view str);

		Bus ParseBus(TransportCatalogue& catalogue, std::string_view str);

		std::vector<Distance> ParseDistance(TransportCatalogue& catalogue, std::string str);

		void Input(TransportCatalogue& catalogue);

	}//��������� ������������ ��� input
}//��������� ������������ ��� transport_catalogue
