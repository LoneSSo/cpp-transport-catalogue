#include "json_reader.h"

#include <fstream>

int main() {

    TransportCatalogue catalogue;
    renderer::MapRenderer renderer;

    RequestHandler handler_(catalogue, renderer);
    reader::JsonReader json_reader(catalogue, handler_, std::cin);
    renderer.SetContext(json_reader.GetRenderContext());
    
    handler_.RenderMap().Render(std::cout);
    json_reader.PrintStat(std::cout);
}