#include "transport_router.h"
using namespace router;

RoutePreBuilder::RoutePreBuilder(const TransportCatalogue& db)
: db_(db){
}

const std::vector<Edge<double>>& RoutePreBuilder::GetAllEdges() const {
    return all_possible_edges_;
}

size_t RoutePreBuilder::GetVertexCount() const {
    return db_.GetAllStops().size() * 2;
}

WayItem RoutePreBuilder::GetWayItem(EdgeId edge_id) const {
    if (wait_edges_ids_.count(edge_id)){
        WayItem item = {wait_edges_ids_.at(edge_id)->name,
                "Wait",
                wait_time_,
                0};
        return item;
    }
    if (ride_edges_ids_.count(edge_id)){
        auto bus_info = ride_edges_ids_.at(edge_id);
        WayItem item = {bus_info.bus->name,
                "Bus",
                bus_info.time,
                ride_edges_ids_.at(edge_id).span_count};
        return item;
    }
    throw ("Bad edge_id at GetWayItem()");
}

void RoutePreBuilder::FillGraph(DirectedWeightedGraph<double>& graph){
    for (Edge edge : all_possible_edges_){
        graph.AddEdge(edge);
    }
}

void RoutePreBuilder::BuildData(){
    wait_time_ = db_.GetRouteSettings().wait_time;
    velocity_ = db_.GetRouteSettings().velocity;
    auto all_stops = db_.GetAllStops();
    for (StopPtr stop : all_stops){
        AddStop(stop);
    }
    auto all_buses = db_.GetAllBuses();
    for (BusPtr bus : all_buses){
        AddBus(bus);
    }
}

void RoutePreBuilder::AddStop(StopPtr stop){
    Edge<double> edge = {current_vertex_id_,
                        ++current_vertex_id_,
                        wait_time_};
    Edge<double> reversed_edge = {edge.to,
                                edge.from,
                                0.0};
    wait_edges_ids_[current_edge_id_++] = stop;
    stops_vertexes_[stop] = {edge.from,
                         edge.to};
    all_possible_edges_.push_back(edge);
    all_possible_edges_.push_back(reversed_edge);
    ++current_edge_id_;
    ++current_vertex_id_;
}

void RoutePreBuilder::AddBus(BusPtr bus){
    auto route = bus->route;
    size_t route_size = route.size();

    if (route_size <= 1){
        return;
    }

    auto compute_weight = [this](double dist){
        return (dist / 1000.0) / (velocity_ / 60);
    };
    
    for (size_t from_idx = 0; from_idx < route_size - 1; from_idx++){
        double distance = 0;
        Edge<double> edge;
        for (size_t to_idx = from_idx + 1; to_idx < route_size; to_idx++){
            size_t span_count = to_idx - from_idx;
            distance += db_.GetDistance(route[to_idx - 1], route[to_idx]).road;
            double weight = compute_weight(distance);
            edge = {stops_vertexes_.at(route[from_idx]).inner,
                    stops_vertexes_.at(route[to_idx]).outer,
                    weight};
            ride_edges_ids_[current_edge_id_++] = {bus,
                                                span_count,
                                                weight};
            all_possible_edges_.push_back(edge);
        }
    }
}

RouteBuilder::RouteBuilder(const TransportCatalogue& db)
: db_(db){
}
void RouteBuilder::InitializeGraph() const {
    data_ = new RoutePreBuilder(db_);
    data_->BuildData();
    graph_ptr_ = new DirectedWeightedGraph<double>(data_->GetVertexCount());
    data_->FillGraph(*graph_ptr_);
    router_ptr_ = new Router<double>(*graph_ptr_);
}

bool RouteBuilder::IsReadyToBuild() const {
    return (data_ && graph_ptr_ && router_ptr_);
}

RouteBuilder::~RouteBuilder(){
        delete router_ptr_;
        delete graph_ptr_;
        delete data_;

}
std::optional<Way> RouteBuilder::GetBestWay(StopPtr from, StopPtr to) const {
    if (!data_->stops_vertexes_.count(from) || !data_->stops_vertexes_.count(to)){
        return std::nullopt;
    }
    VertexId from_id = data_->stops_vertexes_.at(from).outer;
    VertexId to_id = data_->stops_vertexes_.at(to).outer;

    auto way_info = router_ptr_->BuildRoute(from_id, to_id);
    if (!way_info){
        return std::nullopt;
    }
    double total_time = way_info->weight;
    std::vector<WayItem> route;

    auto edges = way_info->edges;
    for (EdgeId edge : edges){
        route.push_back(data_->GetWayItem(edge));
    }
    Way way = {total_time, route};
    return way;
}