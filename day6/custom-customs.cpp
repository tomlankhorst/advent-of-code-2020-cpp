#include <algorithm>
#include <numeric>
#include <set>

#include "day5/arg_input.hpp"
#include "day5/tokenize.hpp"

namespace ranges = std::ranges;

auto main(int argc, char* argv[]) -> int {
  auto file = get_input(argc, argv);

  if (std::holds_alternative<int>(file))
    return std::get<int>(file);

  auto& input = std::get<std::ifstream>(file);

  auto tokens = tokenize(input);

  struct person_t {
    using answers_t = std::set<char>;
    answers_t answers;
  };
  struct group_t {
    std::set<char> any_answered;
    std::set<char> all_answered;
    std::vector<person_t> persons;
  };
  auto groups = std::vector<group_t> {};

  group_t group {};
  for (const auto& l : tokens) {
    if (l.empty()) {
      groups.push_back(group);
      group = {};
      continue;
    }

    auto answers = person_t::answers_t {};
    ranges::copy(l, std::inserter(answers, answers.end()));
    // union
    ranges::copy(l, std::inserter(group.any_answered, group.any_answered.end()));
    // intersection
    if (group.persons.empty()) {
      group.all_answered = answers;
    } else {
      auto intersect = person_t::answers_t {};
      std::set_intersection(group.all_answered.cbegin(), group.all_answered.cend(), answers.cbegin(), answers.cend(), std::inserter(intersect, intersect.end()));
      group.all_answered = intersect;
    }
    group.persons.emplace_back(person_t{answers});
  }
  if (!group.persons.empty())
    groups.push_back(group);

  auto sum = std::accumulate(groups.cbegin(), groups.cend(), 0, [](auto sum, const auto& group){
    return sum + group.any_answered.size();
  });

  std::cout << "Part 1: sum is " << sum << "\n";

  auto sum_p2 = std::accumulate(groups.cbegin(), groups.cend(), 0, [](auto sum, const auto& group){
    return sum + group.all_answered.size();
  });

  std::cout << "Part 2: sum is " << sum_p2 << "\n";
}