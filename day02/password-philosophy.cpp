#include <fstream>
#include <iostream>
#include <vector>
#include <ranges>
#include <algorithm>
#include <regex>

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

  std::vector<std::string> tokens;
  for (std::string line; std::getline(file, line);)
    tokens.push_back(line);

  struct policy_t {
    int min = 0, max = 0;
    char ch = '\0';
  };
  using password_t = std::string;

  struct entry {
    policy_t policy;
    password_t password;
  };

  std::regex regex(R"((\d+)-(\d+)\s(\w): (\w+))");

  std::vector<std::optional<entry>> opt_input (tokens.size());

  std::ranges::transform(tokens, opt_input.begin(), [&regex](const auto& line) -> std::optional<entry> {
    std::smatch match;
    auto matched = std::regex_match(line, match, regex);
    if(!matched || match.size() != 5) {
      return std::nullopt;
    }
    int min = std::stoi(match[1].str());
    int max = std::stoi(match[2].str());
    char ch = match[3].str().begin()[0];
    std::string pass = match[4].str();
    return entry{ {min, max, ch}, pass };
  });

  auto condition_one = [](const std::optional<entry>& eo) {
    if (!eo) return false;
    auto e = *eo;
    auto c_count = std::count_if(e.password.cbegin(), e.password.cend(), [&e](char c){ return c == e.policy.ch; } );
    return c_count >= e.policy.min && c_count <= e.policy.max;
  };

  std::cout << "Valid, part 1: " << std::ranges::count_if(opt_input, condition_one) << std::endl;

  auto condition_two = [](const std::optional<entry>& eo) {
    if (!eo) return false;
    auto e = *eo;
    return (e.password[e.policy.min-1] == e.policy.ch) != (e.password[e.policy.max-1] == e.policy.ch);
  };

  std::cout << "Valid, part 2: " << std::ranges::count_if(opt_input, condition_two) << std::endl;
}
