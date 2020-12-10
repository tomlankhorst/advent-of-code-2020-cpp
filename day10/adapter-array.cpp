#include "day5/arg_input.hpp"

#include <algorithm>
#include <numeric>
#include <ranges>
#include <vector>

namespace ranges = std::ranges;

using tree_t = std::unordered_map<int, std::vector<int>>;
using cache_t = std::unordered_map<size_t, long long>;

auto traverse_impl(const tree_t& tree, int from, int to, cache_t& cache) -> long long {
  auto cache_key = [](int from, int to) {
    return static_cast<size_t>(from) << 32u | static_cast<size_t>(to);
  };

  if (cache.contains(cache_key(from, to))) // if we've seen path (from, to), use the cached value
    return cache.at(cache_key(from, to));

  // check if the tree contains adapters that will lead to `to`
  if (!tree.contains(to))
    return 0;
  const auto& down = tree.at(to);
  // for each adapter that leads to `to`, traverse again. If this is the start, count the path `+1`.
  auto ways = std::accumulate(down.cbegin(), down.cend(), 0ll, [&](auto a, auto i) {
    if (i == from) // if at the end, count this is a possible path
      return a + 1;
    return a + traverse_impl(tree, from, i, cache);
  });

  cache[cache_key(from, to)] = ways;
  return ways;
}

template<typename... Args>
auto traverse(Args... args) {
  cache_t cache;
  return traverse_impl(std::forward<Args>(args)..., cache);
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

  std::vector<long long> adapters;
  for (std::string l; std::getline(input, l); )
    adapters.push_back(std::stoll(l));

  adapters.push_back(0); // throw the 0-jolts outlet in there
  ranges::sort(adapters);
  adapters.push_back(adapters.back()+3); // and the 3-jolts PC

  // in the sorted range, get the difference between adapters
  auto steps = adapters;
  ranges::transform(steps, steps.begin(), [](auto a){
    static auto prev = a;
    auto diff = a - prev;
    prev = a;
    return diff;
  });

  // count the occurence of 1- and 3-difference steps
  auto occurence = [](const auto& it, const auto& cmp){
    return ranges::count_if(it, [&cmp](const auto& v) { return v == cmp; });
  };
  auto one_diff = occurence(steps, 1),
    three_diff = occurence(steps, 3);
  std::cout << "Part 1: " << one_diff << " 1-jolt diffs, " << three_diff << " 3-jolt diffs: " << one_diff * three_diff << "\n";

  // Part 2: make a tree of which adapters lead to adapter i
  auto adapter_tree = tree_t {};
  adapter_tree.reserve(adapters.size());

  for (size_t i = 1; i < adapters.size(); i++) {
    auto adapter = adapters[i];
    auto it = adapters.cbegin() + i;
    // find in the sorted range [i-3, i) the adapter that suits, then all adapters [i-3,i) will suit
    auto least = std::upper_bound(it - std::min(3ul, i), it, adapter - 3 - 1);
    adapter_tree[adapter] = tree_t::value_type::second_type(std::distance(least, it));
    std::copy(least, it, adapter_tree.at(adapter).begin());
  }

  std::cout << "Part 2: " << traverse(adapter_tree, adapters.front(), adapters.back()) << "\n";
}
