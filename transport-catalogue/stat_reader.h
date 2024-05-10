#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace StatReader {
struct RequestDescription{
    std::string type;
    std::string id;
};

void GetData(TransportCatalogue catalogue, std::istream& input, std::ostream& output);

RequestDescription ParseRequest(std::string_view request);

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output);



void PrintStat(const TransportCatalogue& transport_catalogue, 
                const RequestDescription& request, std::ostream& output);

std::ostream& operator<<(std::ostream& output, const BusInfo* info);

std::ostream& operator<<(std::ostream& output, const StopInfo* info);

} //namespace StatReader