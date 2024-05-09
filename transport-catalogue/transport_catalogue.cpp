#include "transport_catalogue.h"

#include <unordered_set>
using namespace std;
using sv = std::string_view;

size_t PairHash::operator()(const pair<const Stop*, const Stop*>& pair) const {
    return sv_hasher_(pair.first -> name) + sv_hasher_(pair.second -> name) * 37;
}

/* 
 * Добавляет маршрут в справочник. 
 * Заполняет информацию о маршруте и 
 * прописывает маршрут в проходные остановки.
 */
void TransportCatalogue::AddBus(sv name, const vector<sv>& route){
    Bus bus{};
    
    bus.name = move(name);
    bus.route = move(route);

    Bus* bus_ptr = &buses_data_.emplace_back(move(bus));
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
 */
void TransportCatalogue::AddStop(sv name, Coordinates coordinates){
    Stop stop;

    stop.name = move(name);
    stop.coordinates = move(coordinates);

    Stop* stop_ptr = &stops_data_.emplace_back(move(stop));
    stops_[stop_ptr->name] = stop_ptr;
    stop_info_[stop_ptr] = {};    
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
 * Прописывает информацию о маршруте в справочнике
 */
void TransportCatalogue::AddBusInfo(const Bus* bus){

    double length = 0;

    pair<Coordinates, Coordinates> from_to_pair;
    // Ничего умнее не придумал.
    unordered_set<sv> uniques;

    bool is_first = true;
    for (auto stop : bus -> route){

        const auto& stop_ = *GetStop(stop);
        uniques.emplace(stop);

        if (is_first){
            from_to_pair.first = stop_.coordinates;
            is_first = false;
            continue;
        }

        from_to_pair.second = stop_.coordinates;
        length += ComputeDistance(from_to_pair.first, from_to_pair.second);
        from_to_pair.first = from_to_pair.second;
    }

    bus_info_[bus] = {bus->route.size(), uniques.size(), length};
}

/*
 * Прописывает сквозные маршруты для остановок.
 */
void TransportCatalogue::AddBusToThroughStops(const Bus* bus){
    for (sv stop : bus->route){

        StopInfo& stop_info = stop_info_[GetStop(stop)];

        stop_info.through_buses.emplace(bus->name);
    }
};



