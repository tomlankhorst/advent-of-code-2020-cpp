#include "arg_input.hpp"
#include "tokenize.hpp"

#include <algorithm>
#include <ranges>

namespace ranges = std::ranges;

auto main(int argc, char* argv[]) -> int {
  auto file = get_input(argc, argv);

  if (std::holds_alternative<int>(file))
    return std::get<int>(file);

  auto &input = std::get<std::ifstream>(file);

  auto tokens = tokenize(input);

  tokens
      .erase(std::remove_if(tokens.begin(), tokens.end(), [](const auto &t) { return t.size() != 10; }), tokens.end());

  struct boarding_pass {
    std::string input;
    uint row, column, seat_id;
  };

  auto boarding_passes = std::vector<boarding_pass>(tokens.size());

  ranges::transform(tokens, boarding_passes.begin(), [](const auto &token) -> boarding_pass {
    uint16_t number = 0;
    ranges::for_each(token, [&number](char c) {
      number <<= 1u;
      number |= (c == 'B' || c == 'R') ? 1u : 0u;
    });
    uint row = (number >> 3u), col = number & 0b111u;
    return {token, row, col, row * 8 + col};
  });

  ranges::sort(boarding_passes, [](const auto &bp1, const auto &bp2) {
    return bp1.seat_id < bp2.seat_id;
  });

  std::cout << "Part 1: highest seat ID: " << boarding_passes.back().seat_id << std::endl;

  uint id = 0;
  for (auto it = boarding_passes.cbegin() + 1; it < boarding_passes.cend(); it++) {
    if (it->seat_id - (it - 1)->seat_id == 2) {
      id = it->seat_id - 1u;
      break;
    }
  }
  std::cout << "Part 2: my ID: " << id << std::endl;

}