#pragma once

#include <string>
#include <string_view>
#include <fstream>
#include <variant>
#include <iostream>

std::variant<int, std::ifstream> get_input(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "usage " << argv[0] << " path-to-input" << std::endl;
    return 0;
  }

  std::string_view path = argv[1];

  auto file = std::ifstream{path.begin()};

  if (!file.good()) {
    std::cerr << "Couldn't read " << path << std::endl;
    return 1;
  }

  return file;
}