#include <iostream>
#include "scene_lua.hpp"
#include "Primitive.hpp"

int main(int argc, char** argv)
{
  std::string filename = "Assets/simple.lua";
  if (argc >= 2) {
    filename = argv[1];
  }

  if (!run_lua(filename)) {
    std::cerr << "Could not open " << filename << std::endl;
    return 1;
  }
}
