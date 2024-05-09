#include "stat_reader.h"

#include <iomanip>
using namespace std;
using namespace std::string_view_literals;

using sv = std::string_view;

RequestDescription ParseRequest(sv request){

    auto space_pos = request.find(' ');
    auto not_space = request.find_first_not_of(' ', space_pos);

    return {string(request.substr(0, space_pos)),
            string(request.substr(not_space, request.size()))};
}

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, sv request,
                       ostream& output) {

    RequestDescription command = ParseRequest(request);
    sv type = command.type;
    sv id = command.id;

    output << type << ' ' << id << ": "sv;

    if (type == "Bus"sv){
        output << transport_catalogue.GetBusInfo(id) << endl;
    }

    if (type == "Stop"sv){
        output << transport_catalogue.GetStopInfo(id) << endl;
    }
}

ostream& operator<<(ostream& output, const BusInfo* info){

    if (info == nullptr){
        output << "not found"sv;
        return output;
    }

    const auto default_precision{static_cast<int>(output.precision())};
    output 
        << info->stops_count << " stops on route, "sv
        << info->unique_count << " unique stops, "sv
        << setprecision(6) << info->route_length << " route length"sv
        << setprecision(default_precision);  

    return output;  
}

ostream& operator<<(ostream& output, const StopInfo* info){

    if (info == nullptr){
        output << "not found"sv;
        return output;
    }

    if (info->through_buses.empty()){
        output << "no buses"sv;
        return output;
    }

    output << "buses "sv;
    bool is_first = true;
    for (auto stop : info->through_buses){
        if (!is_first){
            output << ' ';
        }
        output << stop;
        is_first = false;
    }

    return output;
}