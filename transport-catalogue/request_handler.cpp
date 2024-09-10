#include "request_handler.h"

RequestHandler::RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer, const router::RouteBuilder& router)
: db_(db)
, renderer_(renderer)
, router_(router){
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

std::optional<router::Way> RequestHandler::GetBestWay(const std::string_view& stop_name_from,
                                        const std::string_view& stop_name_to) const {
    if (!router_.IsReadyToBuild()){
        router_.InitializeGraph();
    }
    return router_.GetBestWay(db_.GetStop(stop_name_from),
                              db_.GetStop(stop_name_to));
}

svg::Document RequestHandler::RenderMap() const {
    svg::Document result;

    renderer_.RenderTo(result);
    return result;
}