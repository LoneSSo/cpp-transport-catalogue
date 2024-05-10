#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    TransportCatalogue catalogue;

    InputReader reader;
    reader.LoadData(catalogue, cin);

    StatReader::GetData(catalogue, cin, cout);
}