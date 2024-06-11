#include "json_reader.h"

int main() {

    TransportCatalogue catalogue;
    renderer::MapRenderer renderer;

    RequestHandler handler_(catalogue, renderer);
    reader::JsonReader json_reader(catalogue, handler_, std::cin);
    renderer.SetContext(json_reader.GetRenderContext());

    json_reader.PrintStat(std::cout);
}