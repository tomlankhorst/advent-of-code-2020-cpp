#pragma once

#include <vector>

auto tokenize(auto& file) {
  std::vector<std::string> tokens;
  for (std::string line; std::getline(file, line);)
    tokens.push_back(line);
  return tokens;
}