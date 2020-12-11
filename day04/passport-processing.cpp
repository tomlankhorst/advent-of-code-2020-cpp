#include <cstring>
#include <fstream>
#include <iostream>
#include <regex>
#include <unordered_map>
#include <variant>
#include <vector>
#include <ranges>

namespace ranges = std::ranges;

enum class key_e {
  byr = 0, iyr, eyr, hgt, hcl, ecl, pid, cid
};

using keys_t = std::array<std::pair<const char*, key_e>, 8>;
static constexpr auto keys = keys_t {
    keys_t::value_type {"byr", key_e::byr},
    {"iyr", key_e::iyr},
    {"eyr", key_e::eyr},
    {"hgt", key_e::hgt},
    {"hcl", key_e::hcl},
    {"ecl", key_e::ecl},
    {"pid", key_e::pid},
    {"cid", key_e::cid},
};

constexpr auto ke(const char* k) -> key_e {
  return ranges::find_if_not(keys, [&k](const auto& p){
    return std::strcmp(p.first, k);
  })->second;
};

struct min_max { int min, max; };
struct height_range {
  static constexpr min_max range_cm {150, 193 }, range_in {59, 76 };
};
struct hex_color {};
struct eye_color {
  static constexpr std::array<const char*, 7> in { "amb", "blu", "brn", "gry", "grn", "hzl", "oth" };
};
struct passport_id {};
struct country_id {};

using rule_t = std::variant<min_max, height_range, hex_color, eye_color, passport_id, country_id>;
using rules_t = std::array<std::pair<key_e, rule_t>, 8>;

static constexpr auto rules = rules_t {
    rules_t::value_type {key_e::byr, min_max{1920, 2002}},
    {key_e::iyr, min_max{2010, 2020}},
    {key_e::eyr, min_max{2020, 2030}},
    {key_e::hgt, height_range{}},
    {key_e::hcl, hex_color{}},
    {key_e::ecl, eye_color{}},
    {key_e::pid, passport_id{}},
    {key_e::cid, country_id{}},
};

static constexpr auto rule(key_e e) -> rule_t {
  return ranges::find_if(rules, [&e](const auto& er) { return er.first == e; })->second;
}

// std::visit overload helper from cppreference.com
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

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

  using tokens_t = std::vector<std::vector<std::pair<std::string, std::string>>>;
  auto tokens = tokens_t {};

  tokens_t::value_type line_tokens;
  for (std::string line; std::getline(file, line);) {
    if (line.empty()) {
      tokens.push_back(line_tokens);
      line_tokens.clear();
    }
    std::stringstream line_s { line };
    for (std::string key_val; std::getline(line_s, key_val, ' ');) {
      auto del = key_val.find(':');
      if (del == std::string::npos)
        break;
      line_tokens.push_back(tokens_t::value_type::value_type {
        std::string_view {key_val.c_str(), del},
        std::string_view {key_val.c_str()+del+1}});
    }
  }
  if (!line_tokens.empty())
    tokens.push_back(line_tokens);

  // part 1
  auto p1_validator = [](const auto& p) {
    for (auto ke : keys) {
      if (ke.second == key_e::cid)
        continue; // don't need to validate
      if (ranges::find_if(p, [&ke](const auto& kv){
        return kv.first == ke.first;
      }) == p.cend())
        return false;
    }
    return true;
  };
  std::cout << "Part 1: " << ranges::count_if(tokens, p1_validator) << " valid\n";

  // part 2
  auto p2_validator = [&p1_validator](const auto& p){
    if (!p1_validator(p))
      return false;

    for (const auto& kv : p) {
      std::string v = kv.second;
      auto e = ke(kv.first.c_str());
      auto r = rule(e);
      if(!std::visit(overloaded {
          [](auto) { return true; }, // unknown rules automatically pass
          [&v](min_max r) {
            auto i = std::stoi(v);
            return i >= r.min && i <= r.max;
          },
          [&v](height_range r) {
            auto in_pos = v.find("in"), cm_pos = v.find("cm");
            auto exp = v.size()-2; // expect in or cm to be the last two chars
            if (in_pos != exp && cm_pos != exp)
              return false;
            auto range = in_pos != std::string::npos ? height_range::range_in : height_range::range_cm;
            auto i = std::stoi(std::string {v.c_str(), in_pos != std::string::npos ? in_pos : cm_pos});
            return i >= range.min && i <= range.max;
          },
          [&v](hex_color r) {
            return std::regex_match(v, std::regex{"^#[0-9,a-f]{6}$"});
          },
          [&v](eye_color r) {
            return ranges::find(eye_color::in, v) != eye_color::in.cend();
          },
          [&v](passport_id r) {
            return std::regex_match(v, std::regex{"^[0-9]{9}$"});
          }
      }, r)) { return false; }
    }
    return true;
  };
  std::cout << "Part 2: " << ranges::count_if(tokens, p2_validator) << " valid\n";

}