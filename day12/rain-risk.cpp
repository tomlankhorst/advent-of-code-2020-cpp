#include "day05/arg_input.hpp"
#include "day05/tokenize.hpp"

#include <algorithm>
#include <ranges>

namespace ranges = std::ranges;

enum class compass_e : char {
  North = 'N',
  East = 'E',
  South = 'S',
  West = 'W'
};

using coord_t = std::pair<int, int>;

enum class dir_e : char {
  Forward = 'F',
  Left = 'L',
  Right = 'R',
};

static constexpr auto compass = std::array<compass_e, 4> {
    compass_e::North,
    compass_e::East,
    compass_e::South,
    compass_e::West,
};

static constexpr auto compass_dir = std::array<coord_t, 4> {
    coord_t {0, 1},
    {1, 0},
    {0, -1},
    {-1, 0},
};

static constexpr auto compass_direction(compass_e e) {
  return compass_dir[ranges::distance(compass.cbegin(), ranges::find(compass, e))];
};

static constexpr auto compass_relative(compass_e e, dir_e d, int steps = 1) {
  int i = ranges::distance(compass.cbegin(), ranges::find(compass, e));
  int size = compass_dir.size();
  switch (d) {
    case dir_e::Left: i-=steps; break;
    case dir_e::Right: i+=steps; break;
    default: break;
  }
  i = (i + size) % size;
  return compass[i];
};

struct ship {
  compass_e dir = compass_e::East;
  coord_t loc = {0,0};
  coord_t waypoint = {10, 1};

  int manhattan() const {
    return std::abs(loc.first) + std::abs(loc.second);
  }

  void turn(dir_e d, int degrees) {
    int steps = degrees/90;
    dir = compass_relative(dir, d, steps);
  }

  void move(int i, compass_e c) {
    auto d = compass_direction(c);
    loc.first += d.first * i;
    loc.second += d.second * i;
  }

  void move_waypoint(int i, compass_e c) {
    auto d = compass_direction(c);
    waypoint.first += d.first * i;
    waypoint.second += d.second * i;
  }

  void follow_waypoint(int i) {
    loc.first += i * waypoint.first;
    loc.second += i * waypoint.second;
  }

  void rotate_waypoint(int degrees) {
    int steps = 4 + (degrees/90);
    for (int i = 0; i < steps; i++)
      waypoint = {waypoint.second, -waypoint.first};
  }
};

std::ostream& operator<< (std::ostream& os, const ship& s) {
  auto x = s.loc.first, y = s.loc.second;
  os << "Coord (" << x << "," << y << "), "
        "Dir " << static_cast<char>(s.dir) << ", "
        "Manhattan: " << s.manhattan() << ", "
        "Waypoint (" << s.waypoint.first << "," << s.waypoint.second << ")";
  return os;
}

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

  auto tokens = tokenize(file);

  struct instruction {
    using action_t = std::variant<compass_e, dir_e>;
    action_t action;
    int value;
  };

  using instructions_t = std::vector<instruction>;
  auto instructions = instructions_t (tokens.size());

  ranges::transform(tokens, instructions.begin(), [](const auto& line) {
    instruction::action_t action;
    switch (line[0]) {
    case 'F': case 'L': case 'R':
      action = static_cast<dir_e>(line[0]); break;
    case 'N': case 'E': case 'S': case 'W':
      action = static_cast<compass_e>(line[0]); break;
    }
    return instruction {
        .action = action,
        .value = std::stoi(line.c_str()+1),
    };
  });

  auto navigate_ship = [](ship s, const instructions_t& is) {
    ranges::for_each(is, [&s](const auto& i){
      if (std::holds_alternative<dir_e>(i.action)) {
        auto d = std::get<dir_e>(i.action);
        if (d == dir_e::Forward) {
          s.move(i.value, s.dir);
        } else {
          s.turn(std::get<dir_e>(i.action), i.value);
        }
      } else {
        s.move(i.value, std::get<compass_e>(i.action));
      }
    });
    return s;
  };

  auto s = ship {};
  s = navigate_ship(s, instructions);

  std::cout << "Part 1: " << s << "\n";
  int a1 = s.manhattan();

  auto navigate_waypoint = [](ship s, const instructions_t& is) {
    ranges::for_each(is, [&s](const auto& i){
      if (std::holds_alternative<dir_e>(i.action)) {
        auto d = std::get<dir_e>(i.action);
        if (d == dir_e::Forward) {
          s.follow_waypoint(i.value);
        } else {
          s.rotate_waypoint(d == dir_e::Left ? -i.value : i.value);
        }
      } else {
        s.move_waypoint(i.value, std::get<compass_e>(i.action));
      }
    });
    return s;
  };

  s = ship {};
  s = navigate_waypoint(s, instructions);

  std::cout << "Part 2: " << s << "\n";
  int a2 = s.manhattan();

  std::cout << a1 << "\n" << a2 << "\n";
}