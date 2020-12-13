#include "day05/tokenize.hpp"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <ranges>
#include <variant>
#include <numeric>

namespace ranges = std::ranges;

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

  auto earliest = std::stoi(tokens[0]);

  auto timetable = tokens[1];
  auto services = std::vector<std::variant<std::monostate, int>> {};

  int from = 0;
  int pos = timetable.find(',', from);
  for(; pos != std::string::npos; pos = timetable.find(',', from)) {
    auto part = timetable.substr(from, pos-from);
    from = pos + 1;
    if (part[0] == 'x') {
      services.emplace_back(std::monostate{});
    } else {
      services.emplace_back(std::stoi(part));
    }
  }
  services.emplace_back(std::stoi(timetable.substr(from)));

  auto services_mod = std::vector<int> (services.size());
  ranges::transform(services, services_mod.begin(), [&earliest](const auto& service){
    if (std::holds_alternative<int>(service)) {
      int bus = std::get<int>(service);
      return bus - earliest % bus;
    } else {
      return std::numeric_limits<int>::max();
    }
  });

  auto briefest = ranges::min_element(services_mod);
  auto service = std::get<int>(services[std::distance(services_mod.begin(), briefest)]);

  auto a1 = service * *briefest;
  std::cout << "Part 1: bus " << service << ", wait " << a1 << ", multiply: " << a1 << "\n";

  auto ns = std::vector<uint64_t>(services.size());
  ranges::transform(services, ns.begin(), [](const auto& service){
    if (std::holds_alternative<int>(service)) {
      return std::get<int>(service);
    }
    return 1;
  });

  // N = 7*13*1*...
  auto N = std::accumulate(ns.cbegin(), ns.cend(), uint64_t {1}, [](auto a, auto n){
    return a * n;
  });

  auto ys = std::vector<uint64_t>(ns.size());
  ranges::transform(ns, ys.begin(), [&N](auto n){
    return N/n;
  });

  int i = 0;
  auto as = std::vector<uint64_t>(services.size());
  ranges::transform(services, as.begin(), [&i](const auto& service){
    auto a = 0;
    if (std::holds_alternative<int>(service)) {
       a = std::get<int>(service) - i;
    }
    i++;
    return a;
  });

  auto zs = std::vector<uint64_t>(ys.size());
  for(size_t i = 0; i < zs.size(); i++) {
    // EEA
    // Extended Euclidian https://rosettacode.org/wiki/Chinese_remainder_theorem#C.2B.2B
    int64_t a = ys[i], b = ns[i];

    int64_t b0 = b, x0 = 0, x1 = 1;

    if (b==1) {
      zs[i] = 1;
      continue;
    }

    while (a > 1) {
      int64_t q = a/b, amb = a%b;
      a = b;
      b = amb;

      int64_t xqx = x1 - q * x0;
      x1 = x0;
      x0 = xqx;
    }

    if (x1 < 0) {
      x1 += b0;
    }

    zs[i] = x1;
  }

  auto x = uint64_t {0};
  for(size_t i = 0; i < zs.size(); i++) {
    x += as[i] * ys[i] * zs[i];
  }

  auto a2 = x % N;

  std::cout << "Part 2: " << a2 << "\n";

  std::cout << a1 << "\n" << a2 << "\n";
}