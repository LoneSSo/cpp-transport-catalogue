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
 * ��������� ��������� � ����������. ������ � ��� ������������� �������� ���������� � ���������� �� ���������.
 */
void TransportCatalogue::AddStop(sv name, Coordinates coordinates, vector<pair<sv, double>> distances = {}){

    if (GetStop(name)){
        ChangeStopCoordinates(GetStop(name), move(coordinates));
    } else {
        Stop stop;

        stop.name = move(name);
        stop.coordinates = move(coordinates);

        Stop* stop_ptr = &stops_data_.emplace_back(move(stop));
        stops_[stop_ptr->name] = stop_ptr;
        stop_info_[stop_ptr] = {};
    }

    /*
     * �������� �� ������� ��� (���������-���������) � ������� ��������� � ���� ������. 
     * ���� ����� ��������� ��� ���, �� ���������� ��� �� �����, ����� �� ����������� ���������. 
     */ 
    for (auto [destination, road_distance] : distances){
        if (!GetStop(destination)){
            AddStop(destination, {0, 0});   
        }        
        AddRoadDistance(name, destination, road_distance);
    }  
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

        geo_length += ComputeDistance(from_to.first->coordinates, 
                                      from_to.second->coordinates);
        road_length += road_distances_[from_to];

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
};

/*
 * ��������� ��������� �� from �� to. � �������, ���� ������ �������� ��� ���.  
 */
void TransportCatalogue::AddRoadDistance(sv from, sv to, double road_distance){
    auto from_ = GetStop(from)
        , to_ = GetStop(to);

    road_distances_[{from_, to_}] = road_distance;

    if (from_ != to_ && !road_distances_.count({to_, from_})){
        road_distances_[{to_, from_}] = road_distance;
    }
}

/*
 * ��������(!!!) ���������� ������������ ���������. 
 */
void TransportCatalogue::ChangeStopCoordinates(const Stop* stop, Coordinates coordinates){

    auto ptr_ = const_cast<Stop*>(stop); // !!!
    ptr_->coordinates = move(coordinates);
}



