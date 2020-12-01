#include <fstream>
#include <vector>
#include <algorithm>
#include <iostream>
#include <ranges>

auto main(int argc, char* argv[]) -> int {
  namespace ranges = std::ranges;

  constexpr auto twenty20 = 2020;

  if (argc != 2) {
    std::cout << "usage: " << argv[0] << " path-to-input" << std::endl;
    return 0;
  }

  std::string_view path = argv[1];

  auto file = std::ifstream{path.begin()};

  if (!file.good()) {
    std::cout << "Could not open " << path << std::endl;
    return 1;
  }

  std::vector<std::string> tokens;
  for (std::string line; std::getline(file, line);)
    tokens.push_back(line);

  std::vector<int> input (tokens.size());
  std::ranges::transform(tokens, input.begin(), [](const auto& line){
    return std::stoi(line);
  });

  ranges::sort(input);

  struct ij { int i, j; };
  std::vector<ij> matching_pairs;
  // for all items `i`
  for (auto i = input.cbegin(); i < input.cend(); i++) {
    // determine whether `2020-i` is in the input
    auto j = twenty20 - *i;
    if (j >= 0 && std::binary_search(i, input.cend(), j) )
      matching_pairs.emplace_back(ij{*i,j});
  }

  ranges::for_each(matching_pairs, [](const auto& ij){
    auto& [i, j] = ij;
    std::cout << "i: " << i << ", j: " << j << ", i*j: " << i*j << std::endl;
  });

  struct ijk { int i, j, k; };
  std::vector<ijk> matching_triples;
  // for all items `i`
  for (auto i = input.cbegin(); i < input.cend(); i++) {
    // find the upper bound `ub` s.t. `i+j <= 2020`
    // and for all items `j` in the range `[i,ub]`
    auto ub = std::upper_bound(i, input.cend(), twenty20 - *i);
    for (auto j = i; j < ub; j++) {
      // determine whether 2020-i-j is in the list
      auto k = twenty20 - *i - *j;
      if (k >= 0 && std::binary_search(j, ub, k))
        matching_triples.emplace_back(ijk{*i, *j, k});
    }
  }

  ranges::for_each(matching_triples, [](const auto& ijk){
    auto& [i,j,k] = ijk;
    std::cout <<
              "i: " << i << ", " <<
              "j: " << j << ", " <<
              "k: " << k << ", " <<
              "i*j*k: " << i*j*k << std::endl;
  });

}
