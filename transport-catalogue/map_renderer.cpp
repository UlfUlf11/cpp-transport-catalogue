#include "map_renderer.h"

namespace map_renderer
{

bool IsZero(double value)
{
    return std::abs(value) < EPSILON;
}

MapRenderer::MapRenderer(RenderSettings& render_settings) : render_settings_(render_settings) {}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const
{
    return { (coords.longitude - min_lon_) * zoom_coeff_ + padding_,
             (max_lat_ - coords.latitude) * zoom_coeff_ + padding_ };
}

SphereProjector MapRenderer::GetSphereProjector(const std::vector<geo::Coordinates>& points) const
{
    return SphereProjector(points.begin(),
                           points.end(),
                           render_settings_.width_,
                           render_settings_.height_,
                           render_settings_.padding_);
}

RenderSettings MapRenderer::GetRenderSettings() const
{
    return render_settings_;
}

int MapRenderer::GetPaletteSize() const
{
    return render_settings_.color_palette_.size();
}

svg::Color MapRenderer::GetColor(int line_number) const
{
    return render_settings_.color_palette_[line_number];
}


void MapRenderer::ConstructPolyline(svg::Polyline& polyline, [[maybe_unused]] int line_number) const
{
    using namespace std::literals;

    polyline.SetStrokeColor(GetColor(line_number));
    polyline.SetFillColor("none"s);
    polyline.SetStrokeWidth(render_settings_.line_width_);
    polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
}

void MapRenderer::ConstructBusTextSubstrate(svg::Text& text, const std::string& name, svg::Point position) const
{

    using namespace std::literals;

    text.SetPosition(position);
    text.SetOffset({ render_settings_.bus_label_offset_.first, render_settings_.bus_label_offset_.second });
    text.SetFontSize(render_settings_.bus_label_font_size_);
    text.SetFontFamily("Verdana");
    text.SetFontWeight("bold");
    text.SetData(name);
    text.SetFillColor(render_settings_.underlayer_color_);
    text.SetStrokeColor(render_settings_.underlayer_color_);
    text.SetStrokeWidth(render_settings_.underlayer_width_);
    text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
}

void MapRenderer::ConstructBusText(svg::Text& text, const std::string& name, int palette, svg::Point position) const
{

    using namespace std::literals;

    text.SetPosition(position);
    text.SetOffset({ render_settings_.bus_label_offset_.first, render_settings_.bus_label_offset_.second });
    text.SetFontSize(render_settings_.bus_label_font_size_);
    text.SetFontFamily("Verdana");
    text.SetFontWeight("bold");
    text.SetData(name);
    text.SetFillColor(GetColor(palette));
}

void MapRenderer::ConstructCircle(svg::Circle& circle, svg::Point position) const
{
    using namespace std::literals;

    circle.SetCenter(position);
    circle.SetRadius(render_settings_.stop_radius_);
    circle.SetFillColor("white");
}

void MapRenderer::ConstructStopTextSubstrate(svg::Text& text, const std::string& name, svg::Point position) const
{
    using namespace std::literals;

    text.SetPosition(position);
    text.SetOffset({ render_settings_.stop_label_offset_.first, render_settings_.stop_label_offset_.second });
    text.SetFontSize(render_settings_.stop_label_font_size_);
    text.SetFontFamily("Verdana");
    text.SetData(name);
    text.SetFillColor(render_settings_.underlayer_color_);
    text.SetStrokeColor(render_settings_.underlayer_color_);
    text.SetStrokeWidth(render_settings_.underlayer_width_);
    text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
}

void MapRenderer::ConstructStopText(svg::Text& text, const std::string& name, svg::Point position) const
{
    using namespace std::literals;

    text.SetPosition(position);
    text.SetOffset({ render_settings_.stop_label_offset_.first, render_settings_.stop_label_offset_.second });
    text.SetFontSize(render_settings_.stop_label_font_size_);
    text.SetFontFamily("Verdana");
    text.SetData(name);
    text.SetFillColor("black");
}

}//end namespace map_renderer
