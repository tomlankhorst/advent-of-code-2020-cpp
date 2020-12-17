#include "day05/arg_input.hpp"
#include "day05/tokenize.hpp"

#include <algorithm>
#include <ranges>
#include <numeric>

namespace ranges = std::ranges;

template<size_t Dim, typename T>
struct object {
  using type = std::vector<typename object<Dim-1, T>::type>;
};

template<typename T>
struct object<0, T> {
  using type = T;
};

template<size_t Dim, typename T>
using object_t = typename object<Dim, T>::type;

template<typename T>
using line_t = object_t<1, T>;
template<typename T>
using rect_t = object_t<2, T>;
template<typename T>
using cube_t = object_t<3, T>;
template<typename T>
using hypercube_t = object_t<4, T>;

template<typename C, typename... Coords>
auto& get(C& object, size_t x, Coords... coords) {
  if constexpr (sizeof...(Coords)) {
    return get(object[x], coords...);
  } else {
    return object[x];
  }
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const line_t<T>& line) {
  ranges::for_each(line, [&os](T ch){ os << ch << " "; });
  return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const cube_t<T>& cube) {
  int z = cube.size() / 2;
  int i = 0;
  ranges::for_each(cube, [&](const auto& rect){
    os << "z=" << (i++)-z << "\n";
    os << rect << "\n";
  });
  return os;
}

rect_t<char> read_rect(const std::vector<std::string>& lines) {
  auto rect = rect_t<char> (lines.size());

  ranges::transform(lines, rect.begin(), [](const std::string& l){
    auto line = line_t<char> (l.size());
    ranges::copy(l, line.begin());
    return line;
  });

  return rect;
}

template<typename T>
size_t count_if(const std::vector<T>& line, const T& val) {
  return ranges::count(line, val);
}

template<typename C, typename T>
size_t count_if(const C& object, const T& val) {
  return std::accumulate(object.cbegin(), object.cend(), size_t{0}, [&](auto a, const auto& element){
    return a + count_if(element, val);
  });
}

template<size_t Dim, typename T>
auto make(size_t N, const T& value) {
  if constexpr (Dim > 1) {
    return object_t<Dim, T>(N, make<Dim-1, T>(N, value));
  } else {
    return object_t<Dim, T>(N, value);
  }
}

template<typename T>
cube_t<T> cubify(const rect_t<T>& in, size_t N) {
  if (N%2!=1)
    throw std::invalid_argument("N must be uneven for the rectangle to be in the mid-Z plane");

  auto w = in.front().size(),
      h = in.size();

  if (w > N || h > N)
    throw std::invalid_argument("Rectangle wider than N");

  auto cube = make<3>(N, '.');

  auto& midplane = cube[N/2];
  int x = (N-w)/2;
  int y = (N-h)/2;
  ranges::for_each(in, [&](const auto& in_line){
    ranges::copy(in_line, midplane[y++].begin()+x);
  });

  return cube;
}

template<typename T>
hypercube_t<T> hypercubify(const rect_t<T>& in, size_t N) {
  auto hcube = make<4>(N, '.');
  hcube[N/2] = cubify<T>(in, N);
  return hcube;
}

template<typename C, typename T>
size_t count_neighbors_impl(const C& object, const T& val, bool mid, int x) {
  int i = 0;
  for (size_t x_ = std::max(0, x-1); x_ < std::min<int>(object.size(), x+2); x_++) {
    if (object.at(x_) == val && !(mid && x == x_)) {
      i++;
    }
  }
  return i;
}

template<typename C, typename T, typename ... Args>
size_t count_neighbors_impl(const C& object, const T& val, bool mid, int x, int y, Args... args) {
  int i = 0;
  for (size_t x_ = std::max(0, x-1); x_ < std::min<int>(object.size(), x+2); x_++) {
    i += count_neighbors_impl(object.at(x_), val, mid && x == x_, y, args...);
  }
  return i;
}

template<typename C, typename T, typename ... Coords>
size_t count_neighbors(const C& object, const T& val, Coords... coords) {
  return count_neighbors_impl(object, val, true, coords...);
}

static constexpr char rule_of_life(char e, int n) {
  return e == '#' ?
         (n==2 || n==3) ? '#' : '.' :
         (n==3) ? '#' : '.';
}

auto evolve(const cube_t<char>& cube) {
  auto neighbors = make<3>(cube.size(), int{0});
  auto evolved = cube;
  auto N = cube.size();
  for (size_t z = 0; z < N; z++) {
    for (size_t y = 0; y < N; y++) {
      for (size_t x = 0; x < N; x++) {
        auto e = get(cube, z, y, x);
        auto& n = get(neighbors, z, y, x);
        n = count_neighbors(cube, '#', z, y, x);
        evolved[z][y][x] = rule_of_life(e,n);
      }
    }
  }
  return std::make_pair(evolved, neighbors);
}

auto evolve(const hypercube_t<char>& hcube) {
  auto neighbors = make<4>(hcube.size(), int{0});
  auto evolved = hcube;
  auto N = hcube.size();
  for (size_t w = 0; w < N; w++) {
    for (size_t z = 0; z < N; z++) {
      for (size_t y = 0; y < N; y++) {
        for (size_t x = 0; x < N; x++) {
          auto& e = get(hcube, w, z, y, x);
          auto& n = get(neighbors, w, z, y, x);
          n = count_neighbors(hcube, '#', w, z, y, x);
          get(evolved, w, z, y, x) = rule_of_life(e,n);
        }
      }
    }
  }
  return std::make_pair(evolved, neighbors);
}

template<typename T>
T evolve_n(T o, int evolutions) {
  for (size_t i = 0; i < evolutions; i++) {
    auto [next, neighbors] = evolve(o);
    o = next;
  }
  return o;
}

auto main(int argc, char* argv[]) -> int {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " {path-to-file}" << std::endl;
    return 1;
  }

  auto file = get_input(argc, argv);

  if (std::holds_alternative<int>(file))
    return std::get<int>(file);

  auto& input = std::get<std::ifstream>(file);

  auto tokens = tokenize(input);

  auto init = read_rect(tokens);

  int evolutions = 6;

  auto cube = cubify<char>(init, init.size()+evolutions*2+1);
  cube = evolve_n(cube, evolutions);

  auto a1 = count_if(cube, '#');
  std::cout << "Part 1: " << a1 << "\n";

  auto hcube = hypercubify<char>(init, init.size()+evolutions*2+1);
  hcube = evolve_n(hcube, evolutions);

  auto a2 = count_if(hcube, '#');
  std::cout << "Part 2: " << a2 << "\n";

  std::cout << a1 << "\n" << a2 << std::endl;
}