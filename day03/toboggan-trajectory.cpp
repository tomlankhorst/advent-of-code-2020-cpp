#include <array>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>

auto main(int argc, char* argv[]) -> int {
  if (argc != 2) {
    std::cout << "usage " << argv[0] << " path-to-input" << std::endl;
    return 0;
  }

  std::string_view path = argv[1];

  auto file = std::ifstream{path.begin()};

  if (!file.good()) {
    std::cout << "Could not open " << path << std::endl;
    return 1;
  }

  // read the map
  static constexpr auto width = 31;
  using row_type = std::array<char, width>;
  std::vector<row_type> map;
  for (std::string line; std::getline(file, line);) {
    row_type row;
    if (line.size() != width) {
      std::cerr << "unexpected row width" << std::endl;
      return 1;
    }
    std::copy_n(line.cbegin(), width, row.begin());
    map.emplace_back(row);
  }

  // Traverse path
  size_t left = 0, trees = 0;
  for(const auto& row : map) {
    if (row[left%width]=='#')
      trees++;
    left += 3;
  }
  std::cout << "Part 1: Encountered " << trees << " trees\n";

  // Traverse and multiply paths
  using right_down_type = std::pair<int, int>;
  uint64_t multiplied = 1;
  for (const auto& rd : { right_down_type {1,1}, {3,1}, {5,1}, {7,1}, {1,2} } ) {
    left = 0, trees = 0;
    for(size_t row = 0; row < map.size(); row += rd.second) {
      if (map[row][left%width]=='#')
        trees++;
      left += rd.first;
    }
    multiplied *= trees;
  }

  if (multiplied)
    std::cout << "Part 2: Multiplied: " << multiplied << "\n";
}
