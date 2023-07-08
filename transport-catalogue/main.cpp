#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"


//сделал следующие пространства имен :
// transport_catalogue {
//     input{}
//     output{}
//     geo{}
//} завершаем пространство имён transport_catalogue


using namespace transport_catalogue;
//using namespace transport_catalogue::input;
//using namespace transport_catalogue::output;

int main() {
    TransportCatalogue catalogue;
    input::Input(catalogue);
    output::Output(catalogue);
}