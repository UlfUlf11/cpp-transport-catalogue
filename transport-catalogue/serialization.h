#pragma once

#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"

#include "svg.h"
#include "svg.pb.h"

#include "map_renderer.h"
#include "map_renderer.pb.h"

#include "transport_router.h"
#include "transport_router.pb.h"

#include <iostream>

namespace serialization
{

struct SerializationSettings
{
    std::string file_name;
};

struct Catalogue
{
    transport_catalogue::TransportCatalogue transport_catalogue_;
    map_renderer::RenderSettings render_settings_;
    transport_catalogue::detail::RoutingSettings routing_settings_;
};

template <typename It>
uint32_t FindStopID(It start, It end, std::string_view name);

void SerializeStops(std::deque<Stop>& stops, transport_catalogue_protobuf::TransportCatalogue& transport_catalogue_proto);
void SerializeBus(const std::deque<Bus>& buses, std::deque<Stop>& stops, transport_catalogue_protobuf::TransportCatalogue& transport_catalogue_proto);

using DistanceMap = std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistanceHasher>;
void SerializeDistances(DistanceMap& distances, std::deque<Stop>& stops, transport_catalogue_protobuf::TransportCatalogue& transport_catalogue_proto);

transport_catalogue_protobuf::TransportCatalogue SerializeCatalogue(const transport_catalogue::TransportCatalogue& transport_catalogue);

transport_catalogue_protobuf::Color SerializeColor(const svg::Color& tc_color);

transport_catalogue_protobuf::RenderSettings SerializeRenderSettings(const map_renderer::RenderSettings& render_settings);

transport_catalogue_protobuf::RoutingSettings SerializeRoutingSettings(const transport_catalogue::detail::RoutingSettings& routing_settings);

void CatalogueSerialization(const transport_catalogue::TransportCatalogue& transport_catalogue,
                            const map_renderer::RenderSettings& render_settings,
                            const transport_catalogue::detail::RoutingSettings& routing_settings,
                            std::ostream& out);


transport_catalogue::TransportCatalogue DeserializeCatalogue(const transport_catalogue_protobuf::TransportCatalogue& transport_catalogue_proto);

svg::Color DeserializeColor(const transport_catalogue_protobuf::Color& color_proto);

map_renderer::RenderSettings DeserializeRenderSettings(const transport_catalogue_protobuf::RenderSettings& render_settings_proto);

transport_catalogue::detail::RoutingSettings DeserializeRoutingSettings(const transport_catalogue_protobuf::RoutingSettings& routing_settings_proto);

Catalogue CatalogueDeserialization(std::istream& in);

}//end namespace serialization
