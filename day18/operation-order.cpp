#include "day05/arg_input.hpp"
#include "day05/tokenize.hpp"

#include <cstdint>
#include <variant>
#include <vector>
#include <string_view>
#include <algorithm>
#include <ranges>
#include <iostream>
#include <numeric>
#include <list>

namespace ranges = std::ranges;

namespace Grammar {
struct Product {};
struct Sum {};
struct Number { uint64_t value; };
struct Paren {};

using Item = std::variant<Product, Sum, Number, Paren>;
}

namespace Parse {
  struct Node;
}
std::ostream& operator<<(std::ostream& os, const Parse::Node& node);

std::ostream& operator<<(std::ostream& os, const Grammar::Item& item) {
  if (std::holds_alternative<Grammar::Product>(item)) {
    os << "*";
  } else if (std::holds_alternative<Grammar::Sum>(item)) {
    os << "+";
  } else if (std::holds_alternative<Grammar::Number>(item)) {
    os << std::get<Grammar::Number>(item).value;
  } else if (std::holds_alternative<Grammar::Paren>(item)) {
    os << "(";
  }
  return os;
}

namespace Lex {
struct Paren { char ch; };
struct Op { char ch; };
struct Num { uint64_t value; };

using Item = std::variant<Paren, Op, Num>;
using Items = std::vector<Item>;

Items lex(std::string_view input) {
  std::vector<Item> items;
  auto it = input.cbegin();
  while (it != input.cend()) {
    auto ch = *it;
    if (ch >= '0' && ch <= '9') {
      auto from = it;
      for(; *it >= '0' && *it <= '9' && it != input.cend(); it++) {}
      items.emplace_back(Num{std::stoull(std::string(from, it))});
    } else if (ch == '+' || ch == '*') {
      items.emplace_back(Op{ch});
      it++;
    } else if (ch == '(' || ch == ')') {
      items.emplace_back(Paren{ch});
      it++;
    } else if (ch == ' ') {
      it++;
    } else {
      throw std::invalid_argument("Unexpected character");
    }
  }
  return items;
}
} // Lex


namespace Parse {
enum class Method { Normal, Flat };
struct Node {
  using Children = std::vector<Node>;
  Grammar::Item item = Grammar::Paren{};
  Children children;
};
std::pair<std::optional<Node>, int> parse_summand(const Lex::Items &tokens, size_t pos);
std::pair<std::optional<Node>, int> parse_term(const Lex::Items &tokens, size_t pos) {
  const auto &token = tokens[pos];
  if (std::holds_alternative<Lex::Num>(token)) {
    return {Parse::Node{Grammar::Number{std::get<Lex::Num>(token).value}}, pos + 1};
  } else if (std::holds_alternative<Lex::Paren>(token)) {
    auto paren = std::get<Lex::Paren>(token);
    if (paren.ch == '(') {
      const auto&[node_paren, next_pos] = parse_summand(tokens, pos + 1);
      // check
      auto closing = tokens[next_pos];
      // check
      if (std::holds_alternative<Lex::Paren>(token) && std::get<Lex::Paren>(closing).ch == ')') {
        return {Node{Grammar::Paren{}, {*node_paren}}, next_pos + 1};
      } else {
        // NOK
      }
    }
  }
  return {std::nullopt, pos};
}

std::pair<std::optional<Node>, int> parse_expr(const Lex::Items &tokens, size_t pos) {
  const auto&[node_summand, next_pos] = parse_term(tokens, pos);
  if (next_pos < tokens.size()) {
    auto token = tokens[next_pos];
    if (std::holds_alternative<Lex::Op>(token) && std::get<Lex::Op>(token).ch == '+') {
      // check
      auto sum = Node{Grammar::Sum{}, {*node_summand}};
      const auto&[rhs, i] = parse_expr(tokens, next_pos + 1);
      // check
      sum.children.push_back(*rhs);
      return {sum, i};
    }
  }
  return {node_summand, next_pos};
}
std::pair<std::optional<Node>, int> parse_summand(const Lex::Items &tokens, size_t pos) {
  const auto&[node_term, next_pos] = parse_expr(tokens, pos);
  if (next_pos < tokens.size()) {
    auto token = tokens[next_pos];
    if (std::holds_alternative<Lex::Op>(token) && std::get<Lex::Op>(token).ch == '*') {
      // check
      auto product = Node{Grammar::Product{}, {*node_term}};
      const auto&[rhs, i] = parse_summand(tokens, next_pos + 1);
      // check
      product.children.push_back(*rhs);
      return {product, i};
    }
  }
  return {node_term, next_pos};
}

std::optional<Node> parse_p2(const Lex::Items &tokens) {
  const auto&[node, pos] = parse_summand(tokens, 0);
  if (pos != tokens.size()) {
    throw std::invalid_argument("Expected end of input");
  }
  return node;
}

std::optional<Node> parse_p1(const Lex::Items& tokens) {
  std::vector<std::optional<Node>> stack(1);

  for (const auto& t : tokens) {
    if (std::holds_alternative<Lex::Num>(t)) {
      auto number = Grammar::Number { std::get<Lex::Num>(t).value };
      if (!stack.back()) {
        stack.back() = Node{number};
      } else {
        stack.back()->children.push_back(Node{number});
      }
    } else if (std::holds_alternative<Lex::Op>(t)) {
      auto& node = *stack.back();
      auto sub = node;
      Grammar::Item item;
      if (std::get<Lex::Op>(t).ch == '+') {
        item = Grammar::Sum{};
      } else {
        item = Grammar::Product{};
      }
      node = Node { item, {sub} };
    } else if (std::holds_alternative<Lex::Paren>(t)) {
      if (std::get<Lex::Paren>(t).ch == '(') {
        stack.emplace_back(std::nullopt);
      } else {
        auto sub = *stack.back();
        stack.pop_back();
        if (!stack.back()) {
          stack.back() = Node{Grammar::Paren{}, {sub}};
        } else {
          stack.back()->children.emplace_back(Node{Grammar::Paren{}, {sub}});
        }
      }
    }
  }

  return stack.back();
}
}

namespace Execute {
uint64_t execute(const Parse::Node& node) {
  auto res = uint64_t {0};
  if (std::holds_alternative<Grammar::Number>(node.item)) {
    res = std::get<Grammar::Number>(node.item).value;
  } else if (std::holds_alternative<Grammar::Sum>(node.item)) {
    res = std::accumulate(node.children.cbegin(), node.children.cend(), uint64_t {0}, [](auto a, const auto& node){
      return a + execute(node);
    });
  } else if (std::holds_alternative<Grammar::Product>(node.item)) {
    res = std::accumulate(node.children.cbegin(), node.children.cend(), uint64_t {1}, [](auto a, const auto& node){
      return a * execute(node);
    });
  } else if (std::holds_alternative<Grammar::Paren>(node.item)) {
    res = execute(node.children.front());
  }
  return res;
}
} // Execute


std::ostream& print_node(std::ostream& os, const Parse::Node& node, size_t indent = 0, char ident_step = 1, char nl = '\n') {
  os << std::string(indent, '\t') << node.item << " (" << Execute::execute(node) << ")" << nl;
  ranges::for_each(node.children, [&](const auto& child){
    print_node(os, child, indent+ident_step, ident_step, nl);
  });
  if (std::holds_alternative<Grammar::Paren>(node.item)) {
    os << std::string(indent, '\t') << ")" << nl;
  }
  return os;
}


std::ostream& operator<<(std::ostream& os, const Parse::Node& node) {
  print_node(os, node);
  return os;
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

  auto l = Lex::lex("2 * 3 + 4");
  auto p = Parse::parse_p2(l);
  std::cout << *p << std::endl;
  std::cout << Execute::execute(*p) << "\n";

  auto sum = std::accumulate(tokens.cbegin(), tokens.cend(), uint64_t {0}, [](auto a, const auto& line) {
    auto l = Lex::lex(line);
    auto p = Parse::parse_p1(l);
    auto r = Execute::execute(*p);
    return a + r;
  });

  std::cout << "Part 1: " << sum << "\n";

  sum = std::accumulate(tokens.cbegin(), tokens.cend(), uint64_t {0}, [](auto a, const auto& line) {
    auto l = Lex::lex(line);
    auto p = Parse::parse_p2(l);
    auto r = Execute::execute(*p);
    return a + r;
  });

  std::cout << "Part 2: " << sum << "\n";

  return 0;
}