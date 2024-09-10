#pragma once

#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include <memory>

class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer, const router::RouteBuilder& router);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusInfo> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через остановку
    const std::set<sv>* GetBusesByStop(const std::string_view& stop_name) const;

    // Возвращает оптимальный маршрут от остановки from до остановки to
    std::optional<router::Way> GetBestWay(const std::string_view& stop_name_from,
                            const std::string_view& stop_name_to) const;

    svg::Document RenderMap() const;
private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
    const router::RouteBuilder& router_;
};
