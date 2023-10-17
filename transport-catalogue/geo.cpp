#define _USE_MATH_DEFINES

#include "geo.h"

#include <cmath>

namespace transport_catalogue
{
    namespace detail
    {
        namespace geo
        {
            double ComputeDistance(Coordinates from, Coordinates to)
            {
                using namespace std;

                if (from == to)
                {
                    return 0;
                }

                static const double dr = M_PI / 180.;

                return acos(sin(from.latitude * dr) * sin(to.latitude * dr)
                    + cos(from.latitude * dr) * cos(to.latitude * dr)
                    * cos(abs(from.longitude - to.longitude) * dr))
                    * 6371000;
            }

        } // end namespace geo
    } // end namespace detail
} // end namespace transport_catalogue
