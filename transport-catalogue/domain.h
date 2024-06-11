#pragma once
#include "geo.h"

#include <set>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>


using sv = std::string_view;

struct Stop {

	std::string name;
	geo::Coordinates coordinates; // from geo.h
};
using StopPtr = const Stop*;

struct Bus {

	std::string name;
	std::vector<StopPtr> route;
	std::vector<StopPtr> edge_stops;
};
using BusPtr = const Bus*;

struct StopInfo {

	std::set<sv> through_buses;
};

struct BusInfo{

	size_t stops_count = 0;
	size_t unique_count = 0;
	double route_length = 0.0;
	double curvature = 0.0;
};

// Хэш только по имени — имена остановок уникальны. 
struct PairHash {
	std::size_t operator()(const std::pair<const Stop*, const Stop*>& pair) const;
private:
	std::hash<sv> sv_hasher_;
};


