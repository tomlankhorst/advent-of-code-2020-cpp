#include "day05/arg_input.hpp"
#include "day05/tokenize.hpp"

#include <algorithm>
#include <ranges>
#include <numeric>

namespace ranges = std::ranges;

using seats_t = std::vector<std::vector<char>>;

std::ostream& operator<<(std::ostream& os, const seats_t& seats) {
  ranges::for_each(seats, [&](const auto& row){
    ranges::for_each(row, [&](auto ch){ os << ch; });
    os << "\n";
  });
  os << "\n";
  return os;
}

auto main(int argc, char* argv[]) -> int{
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

  auto tokens = tokenize(file);

  auto seats = seats_t (tokens.size());
  ranges::transform(tokens, seats.begin(), [](const auto& line) {
    auto row = seats_t::value_type (line.size());
    ranges::copy(line, row.begin());
    return row;
  });

  // per-seat observed occupancy
  struct occupancy {
    size_t empty = 0, floor = 0, occupied = 0;
  };

  // per-seat behavior based on occupancy
  auto behaviors = [](int max_occ = 4) {
    return [max_occ](char seat, occupancy occ) -> char {
      if (seat == 'L' && !occ.occupied) {
        return '#';
      } else if (seat == '#' && occ.occupied >= max_occ) {
        return 'L';
      }
      return seat;
    };
  };

  // part 1, adjacent occupancy strategy
  auto evolve_seat_adjacent = [&behaviors](const seats_t& seats, int row, int col) {
    static auto behavior = behaviors(4);

    auto up_row = std::max(0, row-1), down_row = std::min<int>(seats.size()-1, row+1),
      left_col = std::max(0, col-1), right_col = std::min<int>(seats[row].size()-1, col+1);

    char seat = seats[row][col];
    occupancy occ;
    for (auto r = up_row; r <= down_row; r++) {
      for (auto c = left_col; c <= right_col; c++) {
        if (c == col && r == row)
          continue;
        switch (seats[r][c]) {
          case 'L': occ.empty++;    break;
          case '#': occ.occupied++; break;
          default:  occ.floor++;
        }
      }
    }

    return behavior(seat, occ);
  };

  // part 2, visible occupancy strategy
  auto evolve_seat_visible = [&behaviors](const seats_t& seats, int row, int col) {
    static auto behavior = behaviors(5);
    const auto dir = { -1, 0, 1 };
    occupancy occ;

    int right = seats.size(),
      back = seats.front().size();

    for (auto hor : dir) {
      for (auto ver : dir) {
        if (hor == 0 && ver == 0)
          continue;
        auto look_row = row, look_col = col;
        for (;;) {
          look_row += hor; look_col+= ver;
          if (look_row < 0 || look_row >= right || look_col < 0 || look_col >= back) {
            occ.empty++;
            break;
          }
          auto seat = seats[look_row][look_col];

          if (seat=='#') {
            occ.occupied++;
            break;
          } else if (seat=='L') {
            occ.empty++;
            break;
          }
        }
      }
    }

    return behavior(seats[row][col], occ);
  };

  // Evolve a seat configuration into the next one through specified method
  auto evolve = [](const seats_t& original, const auto& method) {
    seats_t seats = original;
    bool changed = false;
    for (size_t row = 0; row < seats.size(); row++) {
      for (size_t col = 0; col < seats[row].size(); col++) {
        auto seat = seats[row][col];
        seats[row][col] = method(original, row, col);
        changed |= seat != seats[row][col];
      }
    }
    return std::make_tuple(seats, changed);
  };

  // Evolve until stabilizes
  auto stabilize = [&evolve](seats_t seats, auto method){
    int evolutions = 0;
    for (;;) {
      auto [evolution, changed] = evolve(seats, method);
      seats = evolution;
      if (!changed)
        return std::make_tuple(seats, evolutions);
      evolutions++;
    }
  };

  // Seat-state counter
  auto seat_count_if = [](const auto& seats, char state){
    return std::accumulate(seats.cbegin(), seats.cend(), 0, [&state](int acc, const auto& row) {
      return acc + ranges::count_if(row, [&state](char seat){ return seat == state; });
    });
  };

  auto [stable1, evolutions1] = stabilize(seats, evolve_seat_adjacent);
  std::cout << "Part 1: " << seat_count_if(stable1, '#') << " occupied after " << evolutions1 << " evolutions\n";

  auto [stable2, evolutions2] = stabilize(seats, evolve_seat_visible);
  std::cout << "Part 2: " << seat_count_if(stable2, '#') << " occupied after " << evolutions2 << " evolutions\n";
}