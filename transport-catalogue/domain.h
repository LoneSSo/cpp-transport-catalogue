#pragma once
#include "geo.h"

#include <set>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */



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


