#include "json_reader.h"

#include <fstream>

int main() {

    {
    TransportCatalogue catalogue;
    renderer::MapRenderer renderer;
    router::RouteBuilder router(catalogue);

    RequestHandler handler_(catalogue, renderer, router);
    reader::JsonReader json_reader(catalogue, handler_, std::cin);
    renderer.SetContext(json_reader.GetRenderContext());
    
    //handler_.RenderMap().Render(std::cout);
    json_reader.PrintStat(std::cout);
    }
}