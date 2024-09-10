#pragma once

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"

namespace reader{


class JsonReader{
public:
    JsonReader(TransportCatalogue& db, const RequestHandler& handler, std::istream& in);

    void FillCatalogue();
    json::Document MakeOutDocument() const;
    void PrintStat(std::ostream& out);
    renderer::RenderContext GetRenderContext() const;

private:
// Entry______________________
    void ParseEntryRequests();
    void FillStops();
    void AddDistances();
    void FillBuses();
    void SetRoutingInfo();

    std::vector<const json::Node*> bus_parsed_requests_;
    std::vector<const json::Node*> stop_parsed_requests_;
    json::Array temp_requests_;
// Out________________________
    void ParseStatRequests();

// Render_____________________
    void ParseRenderSettings();
    svg::Color ColorAsString(const json::Node& node) const;

// Stuff______________________
    std::vector<StopPtr> MakeRoute(const json::Array& stops, bool is_roundtrip) const;
    json::Node MakeStatNode(const json::Node& request) const;
    json::Array MakeArray(const std::set<sv>* set) const;
    std::vector<StopPtr> GetEdgeStops(const json::Array& stops) const;
    json::Array MakeWayArray(const router::Way& way) const;

    TransportCatalogue& db_;
    RequestHandler handler_;
    json::Document root_request_;

    renderer::RenderContext render_context_;
};





} // namespace reader