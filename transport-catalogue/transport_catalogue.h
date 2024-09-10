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

	std::vector<BusPtr> GetAllBuses() const;
	

	const Stop* AddStop(sv name, geo::Coordinates coordinates);

	const Stop* GetStop(sv name) const;

	const StopInfo* GetStopInfo(sv name) const;

	std::vector<StopPtr> GetAllStops() const;


	void AddDistance(StopPtr from, StopPtr to, double distance);

	geo::Distance GetDistance(StopPtr from, const StopPtr to) const;

	void SetRouteSettings(RouteSettings settings);

	RouteSettings GetRouteSettings() const;

private:
	void AddBusInfo(BusPtr bus);

	void AddBusToThroughStops(BusPtr bus);

	std::deque<Bus> buses_data_;
	std::unordered_map<sv, BusPtr> buses_;	
	std::unordered_map<BusPtr, BusInfo> bus_info_;

	std::deque<Stop> stops_data_;
	std::unordered_map<sv, StopPtr> stops_;
	std::unordered_map<StopPtr, StopInfo> stop_info_;

	DistancesInfo distances_;
	RouteSettings route_settings_;
};
