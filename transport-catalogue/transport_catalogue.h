#pragma once

#include "geo.h"

#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>



struct Stop {

	std::string name;
	Coordinates coordinates; // from geo.h
};

struct Bus {

	std::string name;
	std::vector<std::string_view> route;
};

struct StopInfo {

	std::set<std::string_view> through_buses;
};

struct BusInfo{

	size_t stops_count = 0;
	size_t unique_count = 0;
	double route_length = 0.0;
};



// Хэш только по имени — имена остановок уникальны. 
struct PairHash {
	std::size_t operator()(const std::pair<const Stop*, const Stop*>& pair) const;
private:
	std::hash<std::string_view> sv_hasher_;
};

class TransportCatalogue {
using sv = std::string_view;
public:
	TransportCatalogue() = default;


	void AddBus(sv name, const std::vector<sv>& route);

	const Bus* GetBus(sv name) const;

	const BusInfo* GetBusInfo(sv name) const;
	

	void AddStop(sv name, Coordinates coordinates);

	const Stop* GetStop(sv name) const;

	const StopInfo* GetStopInfo(sv name) const;

private:
	void AddBusInfo(const Bus* bus);

	void AddBusToThroughStops(const Bus* bus);

	std::deque<Bus> buses_data_;
	std::unordered_map<sv, const Bus*> buses_;	
	std::unordered_map<const Bus*, BusInfo> bus_info_;

	std::deque<Stop> stops_data_;
	std::unordered_map<sv, const Stop*> stops_;
	std::unordered_map<const Stop*, StopInfo> stop_info_;
};
