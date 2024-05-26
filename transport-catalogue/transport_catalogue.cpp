#include "transport_catalogue.h"

#include <unordered_set>
using namespace std;
using sv = std::string_view;

size_t PairHash::operator()(const pair<const Stop*, const Stop*>& pair) const {
    return sv_hasher_(pair.first -> name) + sv_hasher_(pair.second -> name) * 37;
}

/* 
 * ��������� ������� � ����������. 
 * ��������� ���������� � �������� � 
 * ����������� ������� � ��������� ���������.
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
 * ���������� ��������� �� �������.
 * ���������� nullptr, ���� ������� �� ������.
 */
const Bus* TransportCatalogue::GetBus(sv name) const {
    if (buses_.count(name)){
        return buses_.at(name);
    }
    return nullptr;
}

/*
 * ���������� ��������� �� ���������� � ��������.
 * ���������� nullptr, ���� ������� �� ������.
 */
const BusInfo* TransportCatalogue::GetBusInfo(sv name) const {
    auto bus = GetBus(name);

    if (bus == nullptr){
        return nullptr;
    }
    
    return &bus_info_.at(bus);
}

/*
 * ��������� ��������� � ����������.
 * ���������� ��������� �� ���������/���������� ���������.
 */
const Stop* TransportCatalogue::AddStop(sv name, Coordinates coordinates){

        Stop stop;

        stop.name = move(name);
        stop.coordinates = move(coordinates);

        Stop* stop_ptr = &stops_data_.emplace_back(move(stop));
        stops_[stop_ptr->name] = stop_ptr;
        stop_info_[stop_ptr] = {};

        return stop_ptr;
}

/*
 * ���������� ��������� �� ���������.
 * ���������� nullptr, ���� ��������� �� �������.
 */
const Stop* TransportCatalogue::GetStop(sv name) const {
    if (stops_.count(name)){
        return stops_.at(name);
    }
    return nullptr;
}

/*
 * ���������� ��������� �� ���������� �� ���������.
 * ���������� nullptr, ���� ��������� �� �������.
 */
const StopInfo* TransportCatalogue::GetStopInfo(sv name) const {
    auto stop = GetStop(name);

    if (stop == nullptr){
        return nullptr;
    }

    return &stop_info_.at(stop);
}

/*
 * ��������� � ���������� ��������� ����� ����������� � ���� ��������� Distance
 * ���������� � geo, � on_road ���������. 
 */
void TransportCatalogue::AddDistance(const Stop* from, const Stop* to, double road_distance){

    double geo_distance = ComputeDistance(from->coordinates, to->coordinates);
    
    distances_[{from, to}] = {geo_distance, road_distance};

    if (from != to && !distances_.count({to, from})){
        distances_[{to, from}] = {geo_distance, road_distance};
    }
}

/*
 * ���������� ��������� Distance, ����������� geo � on_road ���������.
 */
Distance TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const {
    return distances_.at({from, to});
}

/*
 * ����������� ���������� � �������� � �����������
 */
void TransportCatalogue::AddBusInfo(const Bus* bus){

    double geo_length = 0;
    double road_length = 0;

    pair<const Stop*, const Stop*> from_to;
    // ������ ����� �� ��������.
    unordered_set<sv> uniques;

    bool is_first = true;
    for (auto stop : bus -> route){

        const auto stop_ = GetStop(stop);
        uniques.emplace(stop);

        if (is_first){
            from_to.first = stop_;
            is_first = false;
            continue;
        }

        from_to.second = stop_;
        Distance distance = GetDistance(from_to.first, from_to.second);

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
 * ����������� �������� �������� ��� ���������.
 */
void TransportCatalogue::AddBusToThroughStops(const Bus* bus){
    for (sv stop : bus->route){

        StopInfo& stop_info = stop_info_[GetStop(stop)];

        stop_info.through_buses.emplace(bus->name);
    }
}




