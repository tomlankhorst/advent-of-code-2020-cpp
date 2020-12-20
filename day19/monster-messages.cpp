#include "day05/arg_input.hpp"
#include "day05/tokenize.hpp"

#include <algorithm>
#include <set>

namespace ranges = std::ranges;

using opts_t = std::vector<std::vector<size_t>>;
using rules_t = std::unordered_map<size_t, std::variant<std::monostate, char, opts_t>>;

auto validate(std::string_view str, const rules_t& rules, size_t pos = 0, size_t rule_number = 0, size_t nest = 0) -> std::set<size_t> {
  auto rule = rules.at(rule_number);

  if (pos >= str.size() || nest > str.size())
    return {};

  if (std::holds_alternative<char>(rule)) {
    if (str.at(pos) ==  std::get<char>(rule)) {
      return { pos + 1 };
    }
    return {};
  } else {
    std::set<size_t> chains {};
    auto opts = std::get<opts_t>(rule);
    for (const opts_t::value_type& opt : opts) {
      std::set<size_t> option_chains {pos };
      for (const auto& item : opt) {
        std::set<size_t> result_chains {};
        for (const auto& it_pos : option_chains) {
          auto res = validate(str, rules, it_pos, item, std::max(it_pos, nest+1));
          ranges::copy(res, std::inserter(result_chains, result_chains.end()));
        }
        option_chains = result_chains;
      }
      ranges::copy(option_chains, std::inserter(chains, chains.end()));
    }
    return chains;
  }
}

auto validate(const std::vector<std::string>& messages, const rules_t& rules) {
  return ranges::count_if(messages, [&rules](const auto& m){
    return validate(m,rules).contains(m.size());
  });
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

  auto rules = rules_t {};
  auto it = tokens.cbegin();
  for (; !it->empty() && it != tokens.cend(); it++) {
    auto l = *it;
    size_t eq;
    if ((eq = l.find(':')) != std::string::npos) {
      auto i = std::stoul(l.substr(0, eq));
      if (l[eq+2] == '"') {
        rules[i] = l[eq+3];
      } else {
        auto opts = opts_t {};
        size_t from = eq+2;
        for(;;) {
          size_t to = l.find('|', from);
          std::string p;
          if (to == std::string::npos) {
            p = l.substr(from);
          } else {
            p = l.substr(from, to - from - 1);
            from = to + 2;
          }
          opts.emplace_back();

          size_t pos;
          std::string token;
          while ((pos = p.find(' ')) != std::string::npos) {
            opts.back().push_back(std::stoi(p.substr(0, pos)));
            p.erase(0, pos + 1);
          }
          opts.back().push_back(std::stoi(p));

          if (to == std::string::npos)
            break;
        }
        rules[i] = opts;
      }
    }
  }

  std::vector<std::string> messages;
  ranges::copy(it+1, tokens.cend(), std::back_inserter(messages));

  auto a1 = validate(messages, rules);

  // 170 too low, 185 too high, 184 correct
  std::cout << "Part 1: " << a1 << std::endl;

  // rule 8 encoded by (42|42 42|...|42 42 42 42 42)
  // rule 11 encoded by (42 31|42 42 31 31|...)
  auto r8 = opts_t {}, r11 = opts_t {};
  for (size_t i = 0; i < 5; i++) {
    r8.emplace_back();
    r11.emplace_back();
    for (size_t j = 0; j <= i; j++) {
      r8[i].push_back(42);
      r11[i].push_back(42);
    }
    for (size_t j = 0; j <= i; j++)
      r11[i].push_back(31);
  }

  rules[8] = r8;
  rules[11] = r11;

  auto a2 = validate(messages, rules);

  std::cout << "Part 2: " << a2 << std::endl;

  std::cout << a1 << "\n" << a2 << "\n";
}