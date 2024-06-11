#include "map_renderer.h"

using namespace renderer;
/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */
bool renderer::IsZero(double value) {
    return std::abs(value) < EPSILON;
}

MapRenderer::MapRenderer() = default;
MapRenderer::MapRenderer(const RenderContext& data)
:data_(data){
}

void MapRenderer::SetContext(const RenderContext& data){
    data_ = data;
    std::sort(data_.buses_to_draw.begin(), data_.buses_to_draw.end(),
                                    [](BusPtr lhs, BusPtr rhs){return lhs->name < rhs->name;});
}

void MapRenderer::RenderTo(svg::Document& document) const {
    auto all_points = GetAllGeoPoints();
    SphereProjector projector{all_points.begin(), all_points.end(),
                                data_.render_settings.width_, 
                                data_.render_settings.height_,
                                data_.render_settings.padding_};
    AddLines(document, projector);
    AddBusNames(document, projector);
    AddStopCircles(document, projector);
    AddStopNames(document, projector);
}

void MapRenderer::AddLines(svg::Document& document, SphereProjector& projector) const {
    size_t color_index = 0;
    size_t over_index = data_.render_settings.color_palette_.size();
    for (auto bus : data_.buses_to_draw){
        if (bus->route.empty()){
            continue;
        }
        if (color_index == over_index){
            color_index = 0;
        }
        document.Add(MakeRoute(bus, projector, color_index));
        ++color_index;
    }
}

void MapRenderer::AddBusNames(svg::Document& document, SphereProjector& projector) const {
    size_t color_index = 0;
    size_t over_index = data_.render_settings.color_palette_.size();
    for (auto bus : data_.buses_to_draw){
        auto edge_stops = bus->edge_stops;
        if (edge_stops.empty()){
            continue;
        }
        if (color_index == over_index){
            color_index = 0;
        }
        std::string_view bus_name = bus->name;
        svg::Point route_begin = projector(edge_stops.front()->coordinates);
        document.Add(MakeBusUnderlayer(bus_name, route_begin));
        document.Add(MakeBusName(bus_name, route_begin, color_index));
        if (edge_stops.front() != edge_stops.back()){
            svg::Point route_end = projector(edge_stops.back()->coordinates);
            document.Add(MakeBusUnderlayer(bus_name, route_end));
            document.Add(MakeBusName(bus_name, route_end, color_index));
        }
        ++ color_index;
    }
}

void MapRenderer::AddStopCircles(svg::Document& document, SphereProjector& projector) const {
    for (auto stop : data_.all_stops){
        svg::Point center = projector(stop->coordinates);
        document.Add(MakeStopCircle(center));
    }
}

void MapRenderer::AddStopNames(svg::Document& document, SphereProjector& projector) const {
    for (auto stop : data_.all_stops){
        svg::Point point = projector(stop->coordinates);
        document.Add(MakeStopUnderlayer(stop->name, point));
        document.Add(MakeStopName(stop->name, point));
    }
}




svg::Polyline MapRenderer::MakeRoute(BusPtr bus_ptr, SphereProjector& projector, size_t color_index) const {
    svg::Polyline line;
    for (auto stop : bus_ptr->route){
        line.AddPoint(projector(stop->coordinates));
    }

    line.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
         .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
         .SetStrokeColor(data_.render_settings.color_palette_.at(color_index++))
         .SetFillColor(svg::NoneColor)
         .SetStrokeWidth(data_.render_settings.line_width_);
    
    return line;
}

svg::Text MapRenderer::MakeBusUnderlayer(sv bus_name, svg::Point point) const {
    svg::Text text;

    text.SetPosition(point)
        .SetData(std::string(bus_name))
        .SetOffset({data_.render_settings.bus_label_offset_x_, data_.render_settings.bus_label_offset_y_})
        .SetFontSize(static_cast<uint32_t>(data_.render_settings.bus_label_font_size_))
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetFillColor(data_.render_settings.underlayer_color_)
        .SetStrokeColor(data_.render_settings.underlayer_color_)
        .SetStrokeWidth(data_.render_settings.underlayer_width_)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    return text;
}

svg::Text MapRenderer::MakeBusName(sv bus_name, svg::Point point, size_t color_index) const {
    svg::Text text;

    text.SetPosition(point)
        .SetData(std::string(bus_name))
        .SetOffset({data_.render_settings.bus_label_offset_x_, data_.render_settings.bus_label_offset_y_})
        .SetFontSize(static_cast<uint32_t>(data_.render_settings.bus_label_font_size_))
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetFillColor(data_.render_settings.color_palette_.at(color_index));
    return text;
}

svg::Circle MapRenderer::MakeStopCircle(svg::Point center) const {
    svg::Circle circle;
    circle.SetCenter(center)
          .SetRadius(data_.render_settings.stop_radius_)
          .SetFillColor("white");
    return circle;
}
svg::Text MapRenderer::MakeStopUnderlayer(sv stop_name, svg::Point point) const {
    svg::Text text;

    text.SetData(std::string(stop_name))
        .SetPosition(point)
        .SetOffset({data_.render_settings.stop_label_offset_x_, data_.render_settings.stop_label_offset_y_})
        .SetFontSize(static_cast<uint32_t>(data_.render_settings.stop_label_font_size_))
        .SetFontFamily("Verdana")
        .SetFillColor(data_.render_settings.underlayer_color_)
        .SetStrokeColor(data_.render_settings.underlayer_color_)
        .SetStrokeWidth(data_.render_settings.underlayer_width_)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    return text;
}
svg::Text MapRenderer::MakeStopName(sv stop_name, svg::Point point) const {
    svg::Text text;

    text.SetData(std::string(stop_name))
        .SetPosition(point)
        .SetOffset({data_.render_settings.stop_label_offset_x_, data_.render_settings.stop_label_offset_y_})
        .SetFontSize(static_cast<uint32_t>(data_.render_settings.stop_label_font_size_))
        .SetFontFamily("Verdana")
        .SetFillColor("black");

    return text;
}

std::vector<geo::Coordinates> MapRenderer::GetAllGeoPoints() const {
    std::vector<geo::Coordinates> result;
    for (auto stop : data_.all_stops){
        result.emplace_back(stop->coordinates);
    }
    return result;
}
