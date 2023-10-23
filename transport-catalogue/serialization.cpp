#include "serialization.h"

namespace serialization
{

template <typename It>
uint32_t FindStopID(It start, It end, std::string_view name)
{

    auto stop_it = std::find_if(start, end, [&name](const  transport_catalogue::detail::Stop stop)
    {
        return stop.name == name;
    });
    return std::distance(start, stop_it);
}


void SerializeStops(std::deque<Stop>& stops, transport_catalogue_protobuf::TransportCatalogue& transport_catalogue_proto)
{
    int id = 0;
    for (const auto& stop : stops)
    {

        transport_catalogue_protobuf::Stop stop_proto;

        stop_proto.set_id(id);
        stop_proto.set_name(stop.name);
        stop_proto.set_latitude(stop.coordinates.latitude);
        stop_proto.set_longitude(stop.coordinates.longitude);

        *transport_catalogue_proto.add_stops() = std::move(stop_proto);

        ++id;
    }
}


void SerializeBus(const std::deque<Bus>& buses, std::deque<Stop>& stops, transport_catalogue_protobuf::TransportCatalogue& transport_catalogue_proto)
{
    for (const auto& bus : buses)
    {

        transport_catalogue_protobuf::Bus bus_proto;

        bus_proto.set_name(bus.name);

        for (auto stop : bus.stops)
        {
            uint32_t stop_id = FindStopID(stops.cbegin(),
                                          stops.cend(),
                                          stop->name);
            bus_proto.add_stops(stop_id);
        }

        bus_proto.set_is_roundtrip(bus.is_roundtrip);
        bus_proto.set_route_length(bus.route_length);

        *transport_catalogue_proto.add_buses() = std::move(bus_proto);
    }
}


void SerializeDistances(DistanceMap& distances, std::deque<Stop>& stops, transport_catalogue_protobuf::TransportCatalogue& transport_catalogue_proto)
{
    for (const auto& [pair_stops, distance] : distances)
    {

        transport_catalogue_protobuf::Distance distance_proto;

        distance_proto.set_start(FindStopID(stops.cbegin(),
                                            stops.cend(),
                                            pair_stops.first->name));

        distance_proto.set_end(FindStopID(stops.cbegin(),
                                          stops.cend(),
                                          pair_stops.second->name));

        distance_proto.set_distance(distance);

        *transport_catalogue_proto.add_distances() = std::move(distance_proto);
    }
}


transport_catalogue_protobuf::TransportCatalogue SerializeCatalogue(const transport_catalogue::TransportCatalogue& transport_catalogue)
{

    transport_catalogue_protobuf::TransportCatalogue transport_catalogue_proto;

    std::deque<Stop> stops = transport_catalogue.GetStops();
    const std::deque<Bus>& buses = transport_catalogue.GetBuses();
    DistanceMap distances = transport_catalogue.GetDistance();

    SerializeStops(stops, transport_catalogue_proto);
    SerializeBus(buses, stops, transport_catalogue_proto);
    SerializeDistances(distances, stops, transport_catalogue_proto);

    return transport_catalogue_proto;
}


transport_catalogue_protobuf::Color SerializeColor(const svg::Color& tc_color)
{

    transport_catalogue_protobuf::Color color_proto;

    if (std::holds_alternative<std::monostate>(tc_color))
    {
        color_proto.set_none(true);

    }
    else if (std::holds_alternative<svg::Rgb>(tc_color))
    {
        svg::Rgb rgb = std::get<svg::Rgb>(tc_color);

        color_proto.mutable_rgb()->set_red(rgb.red);
        color_proto.mutable_rgb()->set_green(rgb.green);
        color_proto.mutable_rgb()->set_blue(rgb.blue);

    }
    else if (std::holds_alternative<svg::Rgba>(tc_color))
    {
        svg::Rgba rgba = std::get<svg::Rgba>(tc_color);

        color_proto.mutable_rgba()->set_red(rgba.red);
        color_proto.mutable_rgba()->set_green(rgba.green);
        color_proto.mutable_rgba()->set_blue(rgba.blue);
        color_proto.mutable_rgba()->set_opacity(rgba.opacity);

    }
    else if (std::holds_alternative<std::string>(tc_color))
    {
        color_proto.set_string_color(std::get<std::string>(tc_color));
    }

    return color_proto;
}


transport_catalogue_protobuf::RenderSettings SerializeRenderSettings(const map_renderer::RenderSettings& render_settings)
{

    transport_catalogue_protobuf::RenderSettings render_settings_proto;

    render_settings_proto.set_width_(render_settings.width_);
    render_settings_proto.set_height_(render_settings.height_);
    render_settings_proto.set_padding_(render_settings.padding_);
    render_settings_proto.set_line_width_(render_settings.line_width_);
    render_settings_proto.set_stop_radius_(render_settings.stop_radius_);
    render_settings_proto.set_bus_label_font_size_(render_settings.bus_label_font_size_);

    transport_catalogue_protobuf::Point bus_label_offset_proto;
    bus_label_offset_proto.set_x(render_settings.bus_label_offset_.first);
    bus_label_offset_proto.set_y(render_settings.bus_label_offset_.second);

    *render_settings_proto.mutable_bus_label_offset_() = std::move(bus_label_offset_proto);

    render_settings_proto.set_stop_label_font_size_(render_settings.stop_label_font_size_);

    transport_catalogue_protobuf::Point stop_label_offset_proto;
    stop_label_offset_proto.set_x(render_settings.stop_label_offset_.first);
    stop_label_offset_proto.set_y(render_settings.stop_label_offset_.second);

    *render_settings_proto.mutable_stop_label_offset_() = std::move(stop_label_offset_proto);
    *render_settings_proto.mutable_underlayer_color_() = std::move(SerializeColor(render_settings.underlayer_color_));
    render_settings_proto.set_underlayer_width_(render_settings.underlayer_width_);

    const auto& colors = render_settings.color_palette_;
    for (const auto& color : colors)
    {
        *render_settings_proto.add_color_palette_() = std::move(SerializeColor(color));
    }

    return render_settings_proto;
}


transport_catalogue_protobuf::RoutingSettings  SerializeRoutingSettings(const transport_catalogue::detail::RoutingSettings& routing_settings)
{

    transport_catalogue_protobuf::RoutingSettings routing_settings_proto;

    routing_settings_proto.set_bus_wait_time(routing_settings.bus_wait_time);
    routing_settings_proto.set_bus_velocity(routing_settings.bus_velocity);

    return routing_settings_proto;
}


void CatalogueSerialization(const transport_catalogue::TransportCatalogue& transport_catalogue, const map_renderer::RenderSettings& render_settings, const transport_catalogue::detail::RoutingSettings& routing_settings,
                            std::ostream& out)
{

    transport_catalogue_protobuf::Catalogue catalogue_proto;

    transport_catalogue_protobuf::TransportCatalogue transport_catalogue_proto = SerializeCatalogue(transport_catalogue);
    transport_catalogue_protobuf::RenderSettings render_settings_proto = SerializeRenderSettings(render_settings);
    transport_catalogue_protobuf::RoutingSettings routing_settings_proto = SerializeRoutingSettings(routing_settings);

    *catalogue_proto.mutable_transport_catalogue() = std::move(transport_catalogue_proto);
    *catalogue_proto.mutable_render_settings() = std::move(render_settings_proto);
    *catalogue_proto.mutable_routing_settings() = std::move(routing_settings_proto);

    catalogue_proto.SerializePartialToOstream(&out);

}


void DeserializeStops(const transport_catalogue_protobuf::TransportCatalogue& proto_transport_catalogue, transport_catalogue::TransportCatalogue& transport_catalogue)
{
    const auto& proto_stops = proto_transport_catalogue.stops();

    for (const auto& proto_stop : proto_stops)
    {

        transport_catalogue::detail::Stop stop;

        stop.name = proto_stop.name();
        stop.coordinates.latitude = proto_stop.latitude();
        stop.coordinates.longitude = proto_stop.longitude();

        transport_catalogue.AddStop(std::move(stop));
    }
}

void DeserializeDistances(const transport_catalogue_protobuf::TransportCatalogue& proto_transport_catalogue, transport_catalogue::TransportCatalogue& transport_catalogue, const std::deque<Stop>& stops)
{
    const auto& proto_distances = proto_transport_catalogue.distances();

    for (const auto& distance : proto_distances)
    {


        transport_catalogue::detail::Stop* start = transport_catalogue.GetStop(stops[distance.start()].name);
        transport_catalogue::detail::Stop* end = transport_catalogue.GetStop(stops[distance.end()].name);

        int dist = distance.distance();

        transport_catalogue.AddDistance(start, end, dist);
    }
}


void DeserializeBuses(const transport_catalogue_protobuf::TransportCatalogue& proto_transport_catalogue, transport_catalogue::TransportCatalogue& transport_catalogue, const std::deque<Stop>& stops)
{
    const auto& buses_proto = proto_transport_catalogue.buses();

    for (const auto& bus_proto : buses_proto)
    {

        transport_catalogue::detail::Bus tc_bus;

        tc_bus.name = bus_proto.name();

        for (auto stop_id : bus_proto.stops())
        {
            auto name = stops[stop_id].name;
            tc_bus.stops.push_back(transport_catalogue.GetStop(name));
        }

        tc_bus.is_roundtrip = bus_proto.is_roundtrip();
        tc_bus.route_length = bus_proto.route_length();

        transport_catalogue.AddBus(std::move(tc_bus));
    }
}


transport_catalogue::TransportCatalogue DeserializeCatalogue(const transport_catalogue_protobuf::TransportCatalogue& transport_catalogue_proto)
{
    //создаем транспортный каталог в который будем десереализировать
    transport_catalogue::TransportCatalogue transport_catalogue;

    //сперва нужно обработать остановки для дальнейших действий
    DeserializeStops(transport_catalogue_proto, transport_catalogue);
    
    const std::deque<Stop>& stops = transport_catalogue.GetStops();

    DeserializeDistances(transport_catalogue_proto, transport_catalogue, stops);
    DeserializeBuses(transport_catalogue_proto, transport_catalogue, stops);

    return transport_catalogue;
}


svg::Color DeserializeColor(const transport_catalogue_protobuf::Color& color_proto)
{

    svg::Color color;

    if (color_proto.has_rgb())
    {
        svg::Rgb rgb;

        rgb.red = color_proto.rgb().red();
        rgb.green = color_proto.rgb().green();
        rgb.blue = color_proto.rgb().blue();

        color = rgb;

    }
    else if (color_proto.has_rgba())
    {
        svg::Rgba rgba;

        rgba.red = color_proto.rgba().red();
        rgba.green = color_proto.rgba().green();
        rgba.blue = color_proto.rgba().blue();
        rgba.opacity = color_proto.rgba().opacity();

        color = rgba;

    }
    else
    {
        color = color_proto.string_color();
    }

    return color;
}


map_renderer::RenderSettings DeserializeRenderSettings(const transport_catalogue_protobuf::RenderSettings& render_settings_proto)
{

    map_renderer::RenderSettings render_settings;

    render_settings.width_ = render_settings_proto.width_();
    render_settings.height_ = render_settings_proto.height_();
    render_settings.padding_ = render_settings_proto.padding_();
    render_settings.line_width_ = render_settings_proto.line_width_();
    render_settings.stop_radius_ = render_settings_proto.stop_radius_();
    render_settings.bus_label_font_size_ = render_settings_proto.bus_label_font_size_();

    render_settings.bus_label_offset_.first = render_settings_proto.bus_label_offset_().x();
    render_settings.bus_label_offset_.second = render_settings_proto.bus_label_offset_().y();

    render_settings.stop_label_font_size_ = render_settings_proto.stop_label_font_size_();

    render_settings.stop_label_offset_.first = render_settings_proto.stop_label_offset_().x();
    render_settings.stop_label_offset_.second = render_settings_proto.stop_label_offset_().y();

    render_settings.underlayer_color_ = DeserializeColor(render_settings_proto.underlayer_color_());
    render_settings.underlayer_width_ = render_settings_proto.underlayer_width_();

    for (const auto& color_proto : render_settings_proto.color_palette_())
    {
        render_settings.color_palette_.push_back(DeserializeColor(color_proto));
    }

    return render_settings;
}


transport_catalogue::detail::RoutingSettings DeserializeRoutingSettings(const transport_catalogue_protobuf::RoutingSettings& routing_settings_proto)
{

    transport_catalogue::detail::RoutingSettings routing_settings;

    routing_settings.bus_wait_time = routing_settings_proto.bus_wait_time();
    routing_settings.bus_velocity = routing_settings_proto.bus_velocity();

    return routing_settings;
}


Catalogue CatalogueDeserialization(std::istream& in)
{

    transport_catalogue_protobuf::Catalogue catalogue_proto;
    auto success_parsing_catalogue_from_istream = catalogue_proto.ParseFromIstream(&in);

    if (!success_parsing_catalogue_from_istream)
    {
        throw std::runtime_error("cannot parse serialized file");
    }

    return {DeserializeCatalogue(catalogue_proto.transport_catalogue()),
            DeserializeRenderSettings(catalogue_proto.render_settings()),
            DeserializeRoutingSettings(catalogue_proto.routing_settings())};
}

}//end namespace serialization
