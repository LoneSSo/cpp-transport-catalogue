#pragma once


#include "domain.h"

#include <deque>
#include <unordered_map>

class TransportCatalogue {
	
using sv = std::string_view;
using DistancesInfo = std::unordered_map<std::pair<const Stop*, const Stop*>, geo::Distance, PairHash>;

public:
	TransportCatalogue() = default;


	void AddBus(sv name, const std::vector<StopPtr>& route, const std::vector<StopPtr>& edge_stops);

	const Bus* GetBus(sv name) const;

	const BusInfo* GetBusInfo(sv name) const;
	

	const Stop* AddStop(sv name, geo::Coordinates coordinates);

	const Stop* GetStop(sv name) const;

	const StopInfo* GetStopInfo(sv name) const;


	void AddDistance(const Stop* from, const Stop* to, double distance);

	geo::Distance GetDistance(const Stop* from, const Stop* to) const;

private:
	void AddBusInfo(const Bus* bus);

	void AddBusToThroughStops(const Bus* bus);

	std::deque<Bus> buses_data_;
	std::unordered_map<sv, const Bus*> buses_;	
	std::unordered_map<const Bus*, BusInfo> bus_info_;

	std::deque<Stop> stops_data_;
	std::unordered_map<sv, const Stop*> stops_;
	std::unordered_map<const Stop*, StopInfo> stop_info_;

	DistancesInfo distances_;
};
