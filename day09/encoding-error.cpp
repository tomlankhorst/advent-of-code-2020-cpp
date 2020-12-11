#include <algorithm>
#include <vector>
#include <deque>
#include <ranges>
#include <numeric>
#include <span>

#include "day05/arg_input.hpp"

namespace ranges = std::ranges;

auto main(int argc, char* argv[]) -> int {
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " {path-to-file} {preamble length}" << std::endl;
    return 1;
  }

  auto file = get_input(argc, argv);

  if (std::holds_alternative<int>(file))
    return std::get<int>(file);

  auto& input = std::get<std::ifstream>(file);

  size_t preamble_len = std::stoi(argv[2]);

  std::vector<uint64_t> numbers;
  for (std::string l; std::getline(input, l); )
    numbers.push_back(std::stoll(l));

  auto preamble = std::deque<uint64_t> (preamble_len);
  ranges::copy_n(numbers.cbegin(), preamble_len, preamble.begin());

  auto failure = std::find_if_not(numbers.cbegin() + preamble_len, numbers.cend(), [&preamble](auto number){
    bool present = false;
    for (size_t i = 0; i < preamble.size(); i++)
      for (size_t j = i+1; j < preamble.size(); j++)
        if (preamble[i] + preamble[j] == number)
          present = true;
    preamble.pop_front();
    preamble.push_back(number);
    return present;
  });

  if (failure != numbers.cend()) {
    std::cout << "Part 1: first failure " << *failure << "\n";
  } else {
    return 0;
  }

  for (size_t i = 0; i < numbers.size(); i++) {
    for (size_t j = i+2; j < numbers.size(); j++) {
      auto span = std::span(numbers.cbegin() + i, numbers.cbegin() + j);
      auto sum = std::accumulate(std::cbegin(span), std::cend(span), 0ull);
      if (sum > *failure) {
        break;
      } else if (sum == *failure) {
        std::cout << "Part 2: range " << i << "-" << j-1 << ", ";
        auto [min,max] = ranges::minmax_element(span);
        std::cout << "min+max: " << *min << "+" << *max << "=" << (*min+*max) << "\n";
      }
    }
  }
}