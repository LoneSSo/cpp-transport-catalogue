#include "json_reader.h"
#include "json_builder.h"

#include <algorithm>
#include <sstream>

using namespace reader;
using namespace std;

using namespace literals;

//--------------------- JsonReader ------------------------

JsonReader::JsonReader(TransportCatalogue& db, const RequestHandler& handler, istream& in)
: db_(db)
, handler_(handler)
, root_request_(json::Load(in)){
    FillCatalogue();
    ParseRenderSettings();
}


void JsonReader::FillCatalogue(){
    ParseEntryRequests();
    FillStops();
    AddDistances();
    SetRoutingInfo();
    FillBuses();
    

    temp_requests_.clear();
    ParseStatRequests();
}

json::Document JsonReader::MakeOutDocument() const {
    std::deque<json::Node> for_print;
    for (const auto& request : temp_requests_){
        for_print.emplace_back(std::move(MakeStatNode(request)));
    }
    return json::Document(json::Array{for_print.begin(), for_print.end()});
}

void JsonReader::PrintStat(std::ostream& out) {
    if (temp_requests_.empty()){
        return;
    }
    json::Print(MakeOutDocument(), out);
    out.flush();
}

renderer::RenderContext JsonReader::GetRenderContext() const {
    renderer::RenderContext temp = render_context_;
    std::unordered_set<StopPtr> used_coords;
    for (auto bus : render_context_.buses_to_draw){
        for (auto stop : bus->route){
            used_coords.emplace(stop);
        }
    }
    for (auto stop : used_coords){
        temp.all_stops.emplace_back(stop);
    }
    std::sort(temp.all_stops.begin(), temp.all_stops.end(), 
            [](StopPtr lhs, StopPtr rhs){return lhs->name < rhs->name;});
            
    return temp;
}

// Entry______________________

void JsonReader::ParseEntryRequests(){
    temp_requests_ = std::move(root_request_.GetRoot().AsDict().at("base_requests"s).AsArray());
    for (const auto& node : temp_requests_){
        if (node.AsDict().at("type").AsString()[0] == 'B'){
            bus_parsed_requests_.emplace_back(&node);
            continue;
        }
        stop_parsed_requests_.emplace_back(&node);
    }
}

void JsonReader::FillStops(){
    for (const auto& request : stop_parsed_requests_){
        geo::Coordinates coords{request->AsDict().at("latitude"s).AsDouble(), 
                                request->AsDict().at("longitude"s).AsDouble()};

        db_.AddStop(std::move(request->AsDict().at("name"s).AsString())
                    ,coords);
    }
}

void JsonReader::AddDistances(){
    for (const auto& request : stop_parsed_requests_){
        for (const auto& [to, distance] : request->AsDict().at("road_distances"s).AsDict()){
            db_.AddDistance(db_.GetStop(std::move(request->AsDict().at("name"s).AsString())) 
                            , db_.GetStop(std::move(to))
                            , distance.AsDouble());
        }       
    }
}

void JsonReader::FillBuses(){
    for (const auto& request : bus_parsed_requests_){
        std::string_view bus_name = std::move(request->AsDict().at("name"s).AsString());
        json::Array stops = request->AsDict().at("stops"s).AsArray();
        db_.AddBus(bus_name
                , std::move(MakeRoute(stops, request->AsDict().at("is_roundtrip"s).AsBool()))
                , GetEdgeStops(stops));

        render_context_.buses_to_draw.emplace_back(db_.GetBus(bus_name));
    }
}

void JsonReader::SetRoutingInfo(){
    json::Node route_info = root_request_.GetRoot().AsDict().at("routing_settings");
    RouteSettings settings = {route_info.AsDict().at("bus_wait_time").AsDouble()
                            , route_info.AsDict().at("bus_velocity").AsDouble()};
    db_.SetRouteSettings(settings);
}

// Out________________________
void JsonReader::ParseStatRequests() {
    if (root_request_.GetRoot().AsDict().count("stat_requests")){
        temp_requests_ = root_request_.GetRoot().AsDict().at("stat_requests"s).AsArray();
    }
}
// Renderer___________________

void JsonReader::ParseRenderSettings(){
    if(!root_request_.GetRoot().AsDict().count("render_settings"s)){
        return;
    }
    json::Dict settings = root_request_.GetRoot().AsDict().at("render_settings"s).AsDict();
    renderer::Settings settings_; 
    settings_.width_ = settings.at("width"s).AsDouble();
    settings_.height_ = settings.at("height"s).AsDouble();
    settings_.padding_ = settings.at("padding"s).AsDouble();

    settings_.stop_radius_ = settings.at("stop_radius"s).AsDouble();
    settings_.line_width_ = settings.at("line_width"s).AsDouble();

    settings_.bus_label_font_size_ = settings.at("bus_label_font_size"s).AsDouble();
    settings_.bus_label_offset_x_ = settings.at("bus_label_offset"s).AsArray().at(0).AsDouble();
    settings_.bus_label_offset_y_ = settings.at("bus_label_offset"s).AsArray().at(1).AsDouble();

    settings_.stop_label_font_size_ = settings.at("stop_label_font_size"s).AsDouble();
    settings_.stop_label_offset_x_ = settings.at("stop_label_offset"s).AsArray().at(0).AsDouble();
    settings_.stop_label_offset_y_ = settings.at("stop_label_offset"s).AsArray().at(1).AsDouble();

    settings_.underlayer_color_ = ColorAsString(settings.at("underlayer_color"s));
    settings_. underlayer_width_ = settings.at("underlayer_width"s).AsDouble();

    for (const auto& color : settings.at("color_palette").AsArray()){
        settings_.color_palette_.emplace_back(ColorAsString(color));
    }
    render_context_.render_settings = settings_;
}
svg::Color JsonReader::ColorAsString(const json::Node& node) const {
    if (node.IsString()){
        return node.AsString();
    }

    json::Array rgb = node.AsArray();
    std::stringstream color;
    if (rgb.size() == 3) {
        color << "rgb("sv
            << rgb.at(0).AsInt() << ','
            << rgb.at(1).AsInt() << ','
            << rgb.at(2).AsInt() << ')';
    } else {
        color << "rgba("sv
            << rgb.at(0).AsInt() << ','
            << rgb.at(1).AsInt() << ','
            << rgb.at(2).AsInt() << ','
            << rgb.at(3).AsDouble() << ')';
    }
    
    return color.str();
};
// Stuff______________________
vector<StopPtr> JsonReader::MakeRoute(const json::Array& stops, bool is_roundtrip) const {
    vector<StopPtr> result;
    for (const auto& node : stops){
        result.emplace_back(db_.GetStop(std::move(node.AsString())));
    }
    if (is_roundtrip){
        return result;
    }
    auto temp = result;
    size_t index = temp.size() - 2;
    for (;index != 0; --index){
        result.emplace_back(temp[index]);
    }
    result.emplace_back(temp[0]);
    return result; 
}

json::Node JsonReader::MakeStatNode(const json::Node& request) const {
    json::Node node;

    if (request.AsDict().at("type"s).AsString()[0] == 'B'){
        auto info_ptr = db_.GetBusInfo(std::move(request.AsDict().at("name"s).AsString()));
        if (info_ptr){
            node = json::Builder{}
                    .StartDict()
                        .Key("request_id"s).Value(request.AsDict().at("id"s).AsInt())
                        .Key("curvature"s).Value(info_ptr->curvature)
                        .Key("route_length"s).Value(static_cast<int>(info_ptr->route_length))
                        .Key("stop_count"s).Value(static_cast<int>(info_ptr->stops_count))
                        .Key("unique_stop_count"s).Value(static_cast<int>(info_ptr->unique_count))
                    .EndDict()
                    .Build();
        } else {
            node = json::Builder{}
                    .StartDict()
                        .Key("request_id"s).Value(request.AsDict().at("id"s).AsInt())
                        .Key("error_message"s).Value("not found"s)
                    .EndDict()
                    .Build();
        }
    } else if (request.AsDict().at("type"s).AsString()[0] == 'S'){
        auto stop_stat = handler_.GetBusesByStop(std::move(request.AsDict().at("name").AsString()));
        if (stop_stat){
            node = json::Builder{}
                    .StartDict()
                        .Key("request_id"s).Value(request.AsDict().at("id"s).AsInt())
                        .Key("buses").Value(MakeArray(stop_stat))
                    .EndDict()
                    .Build();
        } else {
            node = json::Builder{}
                    .StartDict()
                        .Key("request_id"s).Value(request.AsDict().at("id"s).AsInt())
                        .Key("error_message"s).Value("not found"s)
                    .EndDict()
                    .Build();
        }
    } else if (request.AsDict().at("type").AsString()[0] == 'R'){
        auto best_way = handler_.GetBestWay(std::move(request.AsDict().at("from").AsString())
                                           ,std::move(request.AsDict().at("to").AsString()));
        if (best_way){
            node = json::Builder{}
                    .StartDict()
                        .Key("request_id"s).Value(request.AsDict().at("id"s).AsInt())
                        .Key("total_time").Value(best_way->total_time)
                        .Key("items").Value(MakeWayArray(*best_way))
                    .EndDict()
                    .Build();
        } else {
            node = json::Builder{}
                    .StartDict()
                        .Key("request_id"s).Value(request.AsDict().at("id"s).AsInt())
                        .Key("error_message"s).Value("not found"s)
                    .EndDict()
                    .Build();
        }
    } else {
        std::stringstream stream;
        handler_.RenderMap().Render(stream);

        node = json::Builder{}
                .StartDict()
                    .Key("request_id"s).Value(request.AsDict().at("id"s).AsInt())
                    .Key("map").Value(stream.str())
                .EndDict()
                .Build();
        stream.clear();
    }
    return node;    
}

json::Array JsonReader::MakeArray(const set<sv>* set) const {
    json::Array result;
    for (const auto& elem : *set){
        result.emplace_back(std::string(elem));
    }
    return result;
} 


json::Array JsonReader::MakeWayArray(const router::Way& way) const {
    json::Array result;
    for (const auto& elem : way.way){
        json::Node temp;
        if (elem.type[0] == 'W'){
            temp = json::Builder{}
                .StartDict()
                    .Key("type").Value(elem.type)
                    .Key("stop_name").Value(std::string(elem.name))
                    .Key("time").Value(elem.time)
                .EndDict()
                .Build();
        } else {
            temp = json::Builder{}
                .StartDict()
                    .Key("type").Value(elem.type)
                    .Key("bus").Value(std::string(elem.name))
                    .Key("time").Value((elem.time))
                    .Key("span_count").Value(static_cast<int>(elem.span_count))
                .EndDict()
                .Build();
    }
        result.emplace_back(temp);
    }
    return result;
}
std::vector<StopPtr> JsonReader::GetEdgeStops(const json::Array& stops) const {
    if (stops.empty()){
        return{};
    }
    return std::vector<StopPtr>{
                    db_.GetStop(stops.front().AsString()),
                    db_.GetStop(stops.back().AsString())};
}