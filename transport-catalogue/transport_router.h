#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

namespace router{
using namespace graph;

struct RideInfo{
    BusPtr bus;
    size_t span_count;
    double time;
};

struct WayItem{
    sv name;
    std::string type;
    double time;
    size_t span_count;

};

struct Way {
    double total_time;
    std::vector<WayItem> way;
};

class RoutePreBuilder{
public:
    friend class RouteBuilder;

    RoutePreBuilder(const TransportCatalogue& db);
    void BuildData();
    const std::vector<Edge<double>>& GetAllEdges() const;
    size_t GetVertexCount() const;
    WayItem GetWayItem(EdgeId edge_id) const;
    void FillGraph(DirectedWeightedGraph<double>& graph);

    
private:
    void AddStop(StopPtr stop);
    void AddBus(BusPtr bus);

    struct StopVertexes {
        VertexId outer;
        VertexId inner;
    };

    double velocity_ = 0;
    double wait_time_ = 0;
    const TransportCatalogue& db_;

    VertexId current_vertex_id_ = 0;
    EdgeId current_edge_id_ = 0;
    std::unordered_map<EdgeId, StopPtr> wait_edges_ids_;
    std::unordered_map<EdgeId, RideInfo> ride_edges_ids_;
    std::unordered_map<StopPtr, StopVertexes> stops_vertexes_;

    std::vector<Edge<double>> all_possible_edges_;
};

class RouteBuilder{
public:
    RouteBuilder(const TransportCatalogue& db);
    void InitializeGraph() const;
    std::optional<Way> GetBestWay(StopPtr from, StopPtr to) const;

    bool IsReadyToBuild() const;

    ~RouteBuilder();
private:
    const TransportCatalogue& db_;
    mutable RoutePreBuilder* data_ = nullptr;
    mutable graph::DirectedWeightedGraph<double>* graph_ptr_ = nullptr;
    mutable graph::Router<double>* router_ptr_ = nullptr;
    
};
} // namespace router

