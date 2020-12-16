#include "day05/arg_input.hpp"
#include "day05/tokenize.hpp"

#include <vector>
#include <set>
#include <algorithm>
#include <ranges>
#include <numeric>
#include <sstream>

struct puzzle {
  struct field {
    std::string name;
    using range_t = std::pair<uint64_t, uint64_t>;
    std::pair<range_t, range_t> ranges = {{0,0},{0,0}};
  };
  using fields_t = std::vector<field>;
  using ticket_t = std::vector<uint64_t>;
  using tickets_t = std::vector<ticket_t>;
  fields_t fields;
  ticket_t mine;
  tickets_t tickets;
};

puzzle read_input(const std::vector<std::string>& lines) {
  auto puzz = puzzle {};

  auto it = lines.cbegin();
  while (it != lines.cend() && !it->empty()) {
    auto field = puzzle::field {};
    auto pos_colon = it->find(':');
    auto pos_or = it->find(" or ", pos_colon);

    field.name = it->substr(0, pos_colon);
    auto r1 = it->substr(pos_colon+2, pos_or-pos_colon);
    auto r2 = it->substr(pos_or+4);
    auto parse_range = [](const std::string& s) {
      auto dash_pos = s.find('-');
      return std::make_pair(
          std::stoull(s.substr(0, dash_pos)),
          std::stoull(s.substr(dash_pos+1)));
    };

    field.ranges = std::make_pair(parse_range(r1), parse_range(r2));
    puzz.fields.push_back(field);
    it++;
  }

  auto parse_line = [](const std::string& line) {
    std::vector<uint64_t> nums;
    std::stringstream ss(line);
    std::string token;
    for (int i = 0; getline(ss, token, ','); i++)
      nums.push_back(stoull(token));
    return nums;
  };

  for (size_t i = 0; i < 2 && it != lines.cend(); i++, it++) {}
  puzz.mine = parse_line(*it++);

  for (size_t i = 0; i < 2 && it != lines.cend(); i++, it++) {}

  while (it != lines.cend() && !it->empty()) {
    puzz.tickets.push_back(parse_line(*it));
    it++;
  }

  return puzz;
}

auto validity(const puzzle::field & f) {
  std::set<uint64_t> valid;
  for (const auto& r : { f.ranges.first, f.ranges.second }) {
    for (uint64_t i : std::views::iota(r.first, r.second+1))
      valid.emplace(i);
  }
  return valid;
}

auto validity(const puzzle::fields_t & fields) {
  return std::accumulate(fields.cbegin(), fields.cend(), std::set<uint64_t> {}, [](auto set, auto f){
    set.merge(validity(f));
    return set;
  });
}

uint64_t ticket_scanning_error_rate(const puzzle& puzzle) {
  auto valid = validity(puzzle.fields);

  std::vector<uint64_t> rejected;
  for (const auto& t : puzzle.tickets) {
    std::ranges::copy_if(t, std::back_inserter(rejected), [&valid](auto v){
      return !valid.contains(v);
    });
  }

  return std::accumulate(rejected.cbegin(), rejected.cend(), 0);
}

auto valid_tickets(const puzzle& puzzle) {
  auto tickets = puzzle.tickets;
  tickets.push_back(puzzle.mine);
  auto valid = validity(puzzle.fields);
  tickets.erase(std::remove_if(tickets.begin(), tickets.end(), [&valid](const auto& t){
    return std::ranges::any_of(t, [&valid](const auto& v){
      return !valid.contains(v);
    });
  }), tickets.end());
  return tickets;
}

auto resolve_field_order(puzzle puzzle) {
  auto valid_field_ranges = std::vector<decltype(validity(puzzle::field{}))> (puzzle.fields.size());
  std::ranges::transform(puzzle.fields, valid_field_ranges.begin(), [](const auto& f){
    return validity(f);
  });

  auto all_available = std::vector<size_t> (puzzle.fields.size());
  std::iota(all_available.begin(), all_available.end(), 0);
  std::vector<decltype(all_available)> available(puzzle.fields.size());
  std::ranges::fill(available, all_available);

  auto tickets = valid_tickets(puzzle);

  auto reduce_available = [&](size_t i, std::vector<size_t> available){
    available.erase(std::remove_if(available.begin(), available.end(), [&](size_t i_field){
      return std::ranges::any_of(tickets, [&](const auto& t){
        return !valid_field_ranges[i_field].contains(t[i]);
      });
    }), available.end());
    return available;
  };

  puzzle::fields_t resolved (puzzle.fields.size());

  bool complete = false;
  while (!complete) {
    complete = true;
    for (size_t i = 0; i < puzzle.fields.size(); i++) {
      auto available_size = available[i].size();
      if (available_size > 0) {
        complete = false;
        available[i] = reduce_available(i, available[i]);
        if (available[i].size() == 1) {
          resolved[i] = puzzle.fields[available[i].front()];
          std::ranges::for_each(available, [&](auto& a){
            auto pos = std::ranges::find(a, available[i].front());
            if (pos != a.cend()) {
              a.erase(pos);
            }
          });
        } else if (available[i].empty()) {
          throw std::invalid_argument("Reduction removed all options. Not solvable.");
        }
      }
    }
  }

  puzzle.fields = resolved;
  return puzzle;
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

  auto puzzle = read_input(tokens);

  auto a1 = ticket_scanning_error_rate(puzzle);
  std::cout << "Part 1: " << a1 << "\n";

  auto solved = resolve_field_order(puzzle);
  auto a2 = uint64_t {1};
  for (size_t i = 0; i < solved.fields.size(); i++) {
    if (solved.fields[i].name.find("departure") == 0) {
      a2 *= solved.mine[i];
    }
  }

  std::cout << "Part 2: " << a2 << "\n";
  std::cout << a1 << "\n" << a2 << std::endl;
}