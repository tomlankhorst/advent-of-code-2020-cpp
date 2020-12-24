#include <algorithm>
#include <iostream>
#include <list>
#include <ranges>

namespace ranges = std::ranges;

/* I use two data-structures for this problem.
 * a) a list for quick insertion, removal (and splice)
 * b) a map to track the relation between value and list iterator for quick lookup
 * To make this work, I have to make sure that the changes in the list are reflected
 * in the map is as well.
 */
using cups_t = std::list<size_t>;

template<typename C>
using tracking_t = std::unordered_map<typename C::value_type, typename C::iterator>;

auto make_tracking(auto& container) {
  using container_t = std::decay_t<decltype(container)>;
  auto tracking = tracking_t<container_t> {};
  for (auto it = container.begin(); it != container.end(); it++)
    tracking[*it] = it;
  return tracking;
}

auto read_input(std::string_view input) {
  auto cups = cups_t {};
  ranges::transform(input, std::back_inserter(cups), [](char ch){
    return std::stoull(std::string{ch});
  });
  return cups;
}

void move_cups(cups_t& cups, cups_t::iterator& it_current, size_t cup_count, tracking_t<cups_t>& tracking) {
  cups_t picked;

  auto splice_remove = [&tracking, &picked, &cups](auto from, auto to){
    for (auto it = from; it != to; it++)
      tracking.erase(*it);
    picked.splice(picked.end(), cups, from, to);
  };

  auto it_pick_begin = std::next(it_current);
  if (it_pick_begin == cups.end())
    it_pick_begin = cups.begin();

  size_t to_end = 1;
  for (; std::next(it_pick_begin, to_end) != cups.end() && to_end < 3; to_end++) {};

  auto c_current = *it_current;

  // Splice the next items
  splice_remove(it_pick_begin, std::next(it_pick_begin, std::min(size_t{3}, to_end)));
  // Splice from begin if not all items spliced
  if (to_end < 3)
    splice_remove(cups.begin(), std::next(cups.begin(), 3-to_end));

  // find a target
  auto c_target = c_current;
  auto target = cups.end();
  for(;;) {
    c_target = (cup_count + c_target - 2) % cup_count + 1;
    if (tracking.contains(c_target)) {
      target = tracking.at(c_target);
      break;
    }
  }
  auto splice_at = std::next(target);
  cups.splice(splice_at, picked);
  for (auto it = std::next(target); it != splice_at; it++)
    tracking[*it] = it;

  // next current
  it_current = tracking[c_current];
  it_current = std::next(it_current);
  if (it_current == cups.end())
    it_current = cups.begin();
}

std::string cups_order(cups_t cups) {
  auto it = ranges::find(cups, 1);
  std::string order;
  for (size_t i = 0; i < cups.size()-1; i++) {
    it++;
    if (it == cups.cend())
      it = cups.begin();
    order.push_back('0'+*it);
  }
  return order;
}

auto main(int argc, char* argv[]) -> int {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " {puzzle-input}" << std::endl;
    return 1;
  }

  auto cups = read_input(argv[1]);
  auto million_cups = cups;

  auto tracking = make_tracking(cups);
  auto it = cups.begin();
  for (size_t i = 0; i < 100; i++) {
    move_cups(cups, it, cups.size(), tracking);
  }

  auto a1 = cups_order(cups);
  std::cout << "Part 1: " << a1 << "\n";

  for (size_t i = cups.size() + 1; i <= 1000*1000; i++)
    million_cups.push_back(i);

  cups = million_cups;
  tracking = make_tracking(cups);
  it = cups.begin();

  for (size_t i = 0; i < 10*1000*1000; i++)
    move_cups(cups, it, cups.size(), tracking);

  auto cup_one = ranges::find(cups, 1);
  auto a2 = *std::next(cup_one, 1)**std::next(cup_one, 2);
  std::cout << "Part 2: " << a2 << "\n";

  std::cout << a1 << "\n" << a2 << "\n";
}