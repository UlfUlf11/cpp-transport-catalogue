#pragma once

#include <cmath>

namespace transport_catalogue
{
    namespace detail
    {
        namespace geo
        {
            const int EARTH_RADIUS = 6371000;
            const double PI = 3.1415926535;

            struct Coordinates
            {
                double latitude;
                double longitude;

                bool operator==(const Coordinates& other) const
                {
                    return latitude == other.latitude && longitude == other.longitude;
                }

                bool operator!=(const Coordinates& other) const
                {
                    return !(*this == other);
                }
            };

            double ComputeDistance(Coordinates start, Coordinates end);

        } // end namespace geo
    } // end namespace detail
} // end namespace transport_catalogue
