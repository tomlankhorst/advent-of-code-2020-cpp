#include <algorithm>
#include <regex>
#include <string_view>
#include <set>
#include <numeric>

#include "day05/arg_input.hpp"
#include "day05/tokenize.hpp"

namespace ranges = std::ranges;

auto resolve (const std::string& color, const auto& rules) -> size_t {
  auto contains = rules.at(color).contains;
  return std::accumulate(contains.cbegin(), contains.cend(), 1, [&](size_t num, const auto& r){
    return num + r.num * resolve(r.color, rules);
  });
};

auto main(int argc, char* argv[]) -> int {
  auto file = get_input(argc, argv);
  if (std::holds_alternative<int>(file))
    return std::get<int>(file);

  auto& input = std::get<std::ifstream>(file);

  auto tokens = tokenize(input);

  struct rule_t {
    struct spec_t { size_t num; std::string color; };
    std::string bag;
    std::vector<spec_t> contains;
  };

  using rules_t = std::unordered_map<std::string, rule_t>;
  rules_t rules (tokens.size());

  ranges::transform(tokens, std::inserter(rules, rules.end()), [](const auto& s) -> rules_t::value_type {
    auto rule = rule_t {};
    std::smatch match;
    std::regex_match(s, match, std::regex(R"((\w+\ \w+) bags contain (no other bags|.+).)"));
    rule.bag = match[1];
    if (match[2] != "no other bags") {
      std::string contains = match[2];
      auto from = 0, to = 0;
      std::vector<std::string> tokens;
      static const auto delim = std::string_view {", "};
      while ((to = contains.find(delim, from)) != std::string::npos) {
        tokens.push_back(contains.substr(from, to-from));
        from = to + delim.size();
      }
      tokens.push_back(contains.substr(from, contains.find('.') - from));
      rule.contains.resize(tokens.size());
      ranges::transform(tokens, rule.contains.begin(), [](const auto& r) -> rule_t::spec_t {
        auto ws = r.find(' ');
        size_t num = std::stoi(r.substr(0, ws));
        auto ws2 = r.find(' ', ws+1);
        ws2 = r.find(' ', ws2+1);
        return { num, r.substr(ws+1, ws2-ws-1) };
      });
    }
    return {rule.bag, rule};
  });

  std::set<std::string> containers {"shiny gold"};
  bool grew = true;
  while (grew) {
    auto s = containers.size();
    for (const auto& rule : rules) {
      if (ranges::find_if(rule.second.contains, [&](const rule_t::spec_t& s){
        return containers.find(s.color) != containers.cend();
      }) != rule.second.contains.cend()) {
        containers.insert(rule.first);
      }
    }
    grew = containers.size() > s;
  }

  std::cout << "Part 1: " << containers.size()-1 << " colors\n";

  std::cout << "Part 2: " << resolve("shiny gold", rules)-1 << " bags\n";
}