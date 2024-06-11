#include "request_handler.h"

RequestHandler::RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer)
:db_(db)
,renderer_(renderer){
}

std::optional<BusInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    auto ptr = db_.GetBusInfo(bus_name);
    if(ptr){
        return *ptr;
    }
    return std::nullopt;
};

const std::set<sv>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    auto ptr = db_.GetStopInfo(stop_name);
    if (ptr){
        return &ptr->through_buses;
    }
    return nullptr;
}

svg::Document RequestHandler::RenderMap() const {
    svg::Document result;

    renderer_.RenderTo(result);
    return result;
}