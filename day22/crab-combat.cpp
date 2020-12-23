#include "day05/arg_input.hpp"
#include "day05/tokenize.hpp"

#include <ranges>
#include <algorithm>
#include <list>
#include <numeric>
#include <set>
#include <unordered_set>
#include <deque>

namespace ranges = std::ranges;

struct deck_t {
  using cards_t = std::deque<size_t>;
  size_t id = 0;
  cards_t cards;
};
using decks_t = std::unordered_map<size_t, deck_t>;
using game_t = std::pair<deck_t::cards_t, deck_t::cards_t>;
struct game_hasher {
  using hash_type = std::size_t;
  hash_type operator()(const game_t & game) const {
    std::size_t h = 0;
    for (auto g : {game.first, game.second})
      for (auto c : g)
        h ^= std::hash<int>{}(c)  + 0x9e3779b9 + (h << 6u) + (h >> 2u);
    return h;
  }
};

auto read_decks(const std::vector<std::string>& tokens) {
  auto decks = decks_t {};
  auto deck = deck_t {};
  static constexpr auto id_prefix = std::string_view {"Player "};
  ranges::for_each(tokens, [&](const auto& token){
    if (token.empty()) { // make sure to end with an empty line
      if (!deck.cards.empty())
        decks[deck.id] = deck;
      deck = {};
    } else if (token.starts_with(id_prefix)) {
      deck.id = std::stoull(token.substr(id_prefix.size(), token.size() - id_prefix.size()-1));
    } else {
      deck.cards.push_back(std::stoull(token));
    }
  });
  if (!deck.cards.empty())
    throw std::invalid_argument("Unprocessed data, did you end with an empty line and new-line?");
  return decks;
}

auto combat(deck_t::cards_t& p1d, deck_t::cards_t& p2d) {
  while (!p1d.empty() && !p2d.empty()) {
    auto c1 = p1d.front(), c2 = p2d.front();
    p1d.pop_front();
    p2d.pop_front();
    if (c1 > c2) {
      p1d.push_back(c1);
      p1d.push_back(c2);
    } else {
      p2d.push_back(c2);
      p2d.push_back(c1);
    }
  }
}

auto recursive_combat(deck_t::cards_t& first, deck_t::cards_t& second) {
  struct frame_t {
    deck_t::cards_t p1d, p2d;
    std::unordered_set<game_t, game_hasher> hist {};
  };
  auto stack = std::vector<frame_t> { {first, second} };
  struct ret_t { bool c1_wins; };
  auto ret = std::optional<ret_t> { std::nullopt };
  for (;;) {
    auto& ctx = stack.back();

    auto c1 = ctx.p1d.front(), c2 = ctx.p2d.front();

    bool c1_wins;

    if (ret) {
      c1_wins = ret->c1_wins;
      ret = std::nullopt;
    } else {

      // anti-loop check
      auto game = game_t{ctx.p1d, ctx.p2d};
      auto has_loop = ctx.hist.contains(game);

      // check if anyone has won
      if (ctx.p1d.empty() || ctx.p2d.empty() || has_loop) {
        if (stack.size() > 1) {
          stack.pop_back();
          ret = { has_loop || ctx.p2d.empty() }; // return the winner
          continue;
        } else {
          break;
        }
      }

      ctx.hist.insert(game);

      if (ctx.p1d.size() > c1 && ctx.p2d.size() > c2) {
        // recursive pla
        auto next = frame_t {};
        ranges::copy_n(ctx.p1d.cbegin()+1, c1, std::back_inserter(next.p1d));
        ranges::copy_n(ctx.p2d.cbegin()+1, c2, std::back_inserter(next.p2d));
        stack.emplace_back(next);
        continue; // we'll get the result in a later iteration
      } else {
        // regular play
        c1_wins = c1 > c2;
      }
    }

    ctx.p1d.pop_front(); ctx.p2d.pop_front();
    if (c1_wins) {
      ctx.p1d.push_back(c1);
      ctx.p1d.push_back(c2);
    } else {
      ctx.p2d.push_back(c2);
      ctx.p2d.push_back(c1);
    }
  }

  first = stack.back().p1d;
  second = stack.back().p2d;
}

template<typename T>
auto play(decks_t decks, size_t p1, size_t p2, T strategy) {
  auto& p1d = decks.at(p1).cards;
  auto& p2d = decks.at(p2).cards;
  strategy(p1d, p2d);
  return decks;
}

size_t score(decks_t result, size_t p1, size_t p2) {
  auto winner = result.at(p1).cards.empty() ? p2 : p1;
  const auto& winner_deck = result.at(winner).cards;

  auto i = 1;
  return std::accumulate(winner_deck.crbegin(), winner_deck.crend(), uint64_t{0}, [&i](auto a, auto v){
    return a + v * (i++);
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

  auto &input = std::get<std::ifstream>(file);

  auto tokens = tokenize(input);

  auto decks = read_decks(tokens);

  auto result1 = play(decks, 1, 2, combat);
  auto a1 = score(result1, 1, 2);
  std::cout << "Part 1: " << a1 << "\n";

  auto result2 = play(decks, 1, 2, recursive_combat);
  auto a2 = score(result2, 1, 2);
  std::cout << "Part 2: " << a2 << "\n";

  std::cout << a1 << "\n" << a2 << "\n";
}