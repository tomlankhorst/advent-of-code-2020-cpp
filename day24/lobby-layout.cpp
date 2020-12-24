#include "day05/arg_input.hpp"
#include "day05/tokenize.hpp"

#include <map>
#include <algorithm>

namespace ranges = std::ranges;

auto main(int argc, char* argv[]) -> int {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " {path-to-file}" << std::endl;
    return 1;
  }

  auto file = get_input(argc, argv);

  if (std::holds_alternative<int>(file))
    return std::get<int>(file);

  auto &input = std::get<std::ifstream>(file);

  auto tokens = tokenize(input);

  using coord_t = std::pair<int, int>;

  auto ref = coord_t { 0, 0 };

  auto flipped = std::map<coord_t, bool> {};

  enum class dir_e { e, se, sw, w, nw, ne };
  auto dir_coords = std::map<dir_e, coord_t> {
      {dir_e::e, {2, 0}},
      {dir_e::ne, {1, 1}},
      {dir_e::nw, {-1, 1}},
      {dir_e::w, {-2, 0}},
      {dir_e::sw, {-1, -1}},
      {dir_e::se, {1, -1}},
  };

  ranges::for_each(tokens, [&](const std::string& inst) {
    auto pos = ref;
    auto it = inst.cbegin();
    for(;;) {
      dir_e dir;
      auto c1 = *it++;
      switch (c1) {
        case 'e': dir = dir_e::e; break;
        case 'w': dir = dir_e::w; break;
        case 'n':
          switch (*it++) {
            case 'e': dir = dir_e::ne; break;
            case 'w': dir = dir_e::nw; break;
          }
          break;
        case 's':
          switch (*it++) {
            case 'e': dir = dir_e::se; break;
            case 'w': dir = dir_e::sw; break;
          }
          break;
      }

      auto dir_coord = dir_coords.at(dir);
      pos.first += dir_coord.first;
      pos.second += dir_coord.second;

      if (it == inst.cend())
        break;
    }
    if (flipped.contains(pos)) {
      flipped.at(pos) = !flipped.at(pos);
    } else {
      flipped[pos] = true;
    }
  });

  auto a1 = ranges::count_if(flipped, [](const auto& entry) { return entry.second; });
  std::cout << "Part 1: " << a1 << "\n";

  for(size_t day = 0; day < 100; day++) {
    auto black_neighbors = std::map<coord_t, size_t> {};
    ranges::for_each(flipped, [&](const auto& entry){
      auto coord = entry.first;
      if (entry.second) { // if black
        ranges::for_each(dir_coords, [&](const auto& dir_coord){ // add one to all neighbors
          auto set = coord;
          set.first += dir_coord.second.first;
          set.second += dir_coord.second.second;
          if (!black_neighbors.contains(set))
            black_neighbors[set] = 0;
          black_neighbors.at(set)++;
        });
      }
    });


    ranges::for_each(flipped, [&](auto& entry){
      if (entry.second && !black_neighbors.contains(entry.first)) {
        entry.second = false;
      }
    });

    ranges::for_each(black_neighbors, [&](const auto& entry){
      auto black = entry.second;
      auto is_black = flipped.contains(entry.first) && flipped.at(entry.first);
      auto flip = is_black && (black == 0 || black > 2);
      flip |= !is_black && black == 2;
      if (flip) {
        if (!flipped.contains(entry.first))
          flipped[entry.first] = false;
        flipped.at(entry.first) = !flipped.at(entry.first);
      }
    });
  }

  // 5278 too high
  auto a2 = ranges::count_if(flipped, [](const auto& entry) { return entry.second; });
  std::cout << "Part 2: " << a2 << "\n";

  std::cout << a1 << "\n" << a2 << "\n";
}