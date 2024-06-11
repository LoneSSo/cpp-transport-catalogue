#pragma once

#include "domain.h"
#include "geo.h"
#include "svg.h"

#include <algorithm>

namespace renderer{

struct Settings {

    double width_ = 0.0;
    double height_ = 0.0;
    double padding_ = 0.0;

    double stop_radius_ = 0.0;
    double line_width_ = 0.0;

    double bus_label_font_size_ = 0.0;
    double bus_label_offset_x_ = 0.0;
    double bus_label_offset_y_= 0.0;

    double stop_label_font_size_ = 0.0;
    double stop_label_offset_x_ = 0.0;
    double stop_label_offset_y_ = 0.0;

    svg::Color underlayer_color_ = svg::NoneColor;
    double underlayer_width_ = 0;

    std::vector<svg::Color> color_palette_;
};

struct RenderContext{
    renderer::Settings render_settings;
    std::vector<StopPtr> all_stops;
    std::vector<BusPtr> buses_to_draw;
};

inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
public:
    // points_begin � points_end ������ ������ � ����� ��������� ��������� geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // ���� ����� ����������� ����� �� ������, ��������� ������
        if (points_begin == points_end) {
            return;
        }

        // ������� ����� � ����������� � ������������ ��������
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // ������� ����� � ����������� � ������������ �������
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // ��������� ����������� ��������������� ����� ���������� x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // ��������� ����������� ��������������� ����� ���������� y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // ������������ ��������������� �� ������ � ������ ���������,
            // ���� ����������� �� ���
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // ����������� ��������������� �� ������ ���������, ���������� ���
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // ����������� ��������������� �� ������ ���������, ���������� ���
            zoom_coeff_ = *height_zoom;
        }
    }

    // ���������� ������ � ������� � ���������� ������ SVG-�����������
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};


class MapRenderer{
public:

    MapRenderer();
    MapRenderer(const RenderContext& data);

    void SetContext(const RenderContext& data);

    void RenderTo(svg::Document& document) const;

private:
    void AddLines(svg::Document& document, SphereProjector& projector) const;
    void AddBusNames(svg::Document& document, SphereProjector& projector) const;
    void AddStopCircles(svg::Document& document, SphereProjector& projector) const;
    void AddStopNames(svg::Document& document, SphereProjector& projector) const;

    svg::Polyline MakeRoute(BusPtr bus, SphereProjector& projector, size_t color_index) const;
    svg::Text MakeBusUnderlayer(sv bus_name, svg::Point point) const;
    svg::Text MakeBusName(sv bus_name, svg::Point point, size_t color_index) const;
    svg::Circle MakeStopCircle(svg::Point center) const;
    svg::Text MakeStopUnderlayer(sv stop_name, svg::Point point) const;
    svg::Text MakeStopName(sv stop_name, svg::Point point) const;
    std::vector<geo::Coordinates> GetAllGeoPoints() const;
    RenderContext data_;
};

}// namespace renderer