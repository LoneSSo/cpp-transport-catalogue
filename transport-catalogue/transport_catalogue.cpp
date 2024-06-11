#include "transport_catalogue.h"

#include <unordered_set>

using namespace std;

/* 
 * Добавляет маршрут в справочник. 
 * Заполняет информацию о маршруте и 
 * прописывает маршрут в проходные остановки.
 */
void TransportCatalogue::AddBus(sv name, const vector<StopPtr>& route, const vector<StopPtr>& edge_stops){
    Bus bus{};
    
    bus.name = std::move(name);
    bus.route = std::move(route);
    bus.edge_stops = std::move(edge_stops);

    Bus* bus_ptr = &buses_data_.emplace_back(std::move(bus));
    buses_[bus_ptr -> name] = bus_ptr;

    AddBusInfo(bus_ptr);
    AddBusToThroughStops(bus_ptr);
}

/*
 * Возвращает указатель на маршрут.
 * Возвращает nullptr, если маршрут не найден.
 */
const Bus* TransportCatalogue::GetBus(sv name) const {
    if (buses_.count(name)){
        return buses_.at(name);
    }
    return nullptr;
}

/*
 * Возвращает указатель на информацию о маршруте.
 * Возвращает nullptr, если маршрут не найден.
 */
const BusInfo* TransportCatalogue::GetBusInfo(sv name) const {
    auto bus = GetBus(name);

    if (bus == nullptr){
        return nullptr;
    }
    
    return &bus_info_.at(bus);
}

/*
 * Добавляет остановку в справочник.
 * Возвращает указатель на созданную/изменённую остановку.
 */
const Stop* TransportCatalogue::AddStop(sv name, geo::Coordinates coordinates){

        Stop stop;

        stop.name = std::move(name);
        stop.coordinates = std::move(coordinates);

        Stop* stop_ptr = &stops_data_.emplace_back(std::move(stop));
        stops_[stop_ptr->name] = stop_ptr;
        stop_info_[stop_ptr] = {};

        return stop_ptr;
}

/*
 * Возвращает указатель на остановку.
 * Возвращает nullptr, если остановка не найдена.
 */
const Stop* TransportCatalogue::GetStop(sv name) const {
    if (stops_.count(name)){
        return stops_.at(name);
    }
    return nullptr;
}

/*
 * Возвращает указатель на информацию об остановке.
 * Возвращает nullptr, если остановка не найдена.
 */
const StopInfo* TransportCatalogue::GetStopInfo(sv name) const {
    auto stop = GetStop(name);

    if (stop == nullptr){
        return nullptr;
    }

    return &stop_info_.at(stop);
}

/*
 * Добавляет в справочник дистанцию между остановками в виде структуры Distance
 * Доавляются и geo, и on_road дистанция. 
 */
void TransportCatalogue::AddDistance(const Stop* from, const Stop* to, double road_distance){

    double geo_distance = geo::ComputeDistance(from->coordinates, to->coordinates);
    
    distances_[{from, to}] = {geo_distance, road_distance};

    if (from != to && !distances_.count({to, from})){
        distances_[{to, from}] = {geo_distance, road_distance};
    }
}

/*
 * Возвращает структуру Distance, содержающую geo и on_road дистанции.
 */
geo::Distance TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const {
    return distances_.at({from, to});
}

/*
 * Прописывает информацию о маршруте в справочнике
 */
void TransportCatalogue::AddBusInfo(const Bus* bus){

    double geo_length = 0;
    double road_length = 0;

    pair<const Stop*, const Stop*> from_to;
    // Ничего умнее не придумал.
    unordered_set<sv> uniques;

    bool is_first = true;
    for (auto stop : bus -> route){

        uniques.emplace(stop->name);

        if (is_first){
            from_to.first = stop;
            is_first = false;
            continue;
        }

        from_to.second = stop;
        geo::Distance distance = GetDistance(from_to.first, from_to.second);

        geo_length += distance.geo;
        road_length += distance.road;

        from_to.first = from_to.second;
    }
    double curvature = road_length / geo_length;

    bus_info_[bus] = {bus->route.size()
                    , uniques.size()
                    , road_length
                    , curvature};
}

/*
 * Прописывает сквозные маршруты для остановок.
 */
void TransportCatalogue::AddBusToThroughStops(BusPtr bus){
    for (StopPtr stop : bus->route){

        StopInfo& stop_info = stop_info_[stop];

        stop_info.through_buses.emplace(bus->name);
    }
}
