#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>

using namespace std;

struct FromToStopDistance {

    string_view from_name;
    string_view to_name;
    double distance;
};
/**
 * Принимает имя from и строку вида "0000m to StopName", возвращает структуру (from, to, дистанция);
 */
FromToStopDistance ParseDistanceToStop(string_view from, string_view command){
    double distance;
    string_view to;

    size_t space_pos = command.find_first_of(' ');
    if (space_pos == command.npos){
        return {};
    }

    size_t name_pos = command.find_first_of('o') + 2;
    if (name_pos == command.npos){
        return {};
    }

    distance = stod(string(command.substr(0, space_pos - 1)));
    to = command.substr(name_pos);

    return {from, to, distance};
}

/**
 * Принимает string_view имя from, вектор команд, парсит строки вида "000m to StopName" и возвращает структуру {from, to, дистанция}
 */
vector<FromToStopDistance> MakeDistancesVector(string_view from, const vector<string>& description){
    if (description.size() <= 2){
        return {};
    }

    vector<FromToStopDistance> result;
    for (size_t i = 2; i < description.size(); i++){
        result.emplace_back(ParseDistanceToStop(from, description[i]));
    }

    return result;
}

/**
 * Принимает вектор команд, объединяет отдельные координаты и возвращает пару координат (широта, долгота)
 */
Coordinates ParseCoordinates(const vector<string>& description) {

    double lat = stod(description[0]);
    double lng = stod(description[1]);

    return {lat, lng};
}

/**
 * Удаляет пробелы в начале и конце строки
 */
string_view Trim(string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
vector<string_view> Split(string_view string, char delim) {
    vector<string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
vector<string_view> ParseRoute(const vector<string>& description) {
    string_view route = description[0];
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    vector<string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    vector<string> description;
    for (auto command : Split(line.substr(colon_pos + 1), ',')){
        description.emplace_back(Trim(command));
    }

    return {string(line.substr(0, space_pos)),
            string(line.substr(not_space, colon_pos - not_space)),
            std::move(description)};
}

void InputReader::LoadData(TransportCatalogue& catalogue, istream& input){
    int base_request_count;
    input >> base_request_count >> ws;

    for (int i = 0; i < base_request_count; ++i) {
        string line;
        getline(cin, line);
        ParseLine(line);
    }
    
    ApplyCommands(catalogue);
};

void InputReader::ParseLine(string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {

    vector<const CommandDescription*> stop_commands;
    vector<const CommandDescription*> bus_commands;
    vector<vector<FromToStopDistance>> distances;

    for (const auto& command : commands_){
        if(command.command[0] == 'S'){
            stop_commands.emplace_back(&command);
            continue;
        }
        bus_commands.emplace_back(&command);
    }

    for (auto command : stop_commands){
        catalogue.AddStop(command->id, ParseCoordinates(command->description));
        distances.emplace_back(MakeDistancesVector(command->id, command->description));
    }

    for (auto vector : distances){
        for(auto distance : vector){
            catalogue.AddDistance(catalogue.GetStop(distance.from_name)
                                , catalogue.GetStop(distance.to_name)
                                , distance.distance);
        }
    }
    
    for (auto command : bus_commands){

        // Избегаем инвалидации string_view.
        vector<string_view> stops_on_route;
        for (auto name : ParseRoute(command->description)){
            stops_on_route.emplace_back(catalogue.GetStop(name)->name);
        }

        catalogue.AddBus(command->id, std::move(stops_on_route));
    }
}