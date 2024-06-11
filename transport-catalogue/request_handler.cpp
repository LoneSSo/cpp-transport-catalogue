#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

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