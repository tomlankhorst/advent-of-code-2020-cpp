#include "day05/arg_input.hpp"
#include "day05/tokenize.hpp"

#include <algorithm>
#include <map>
#include <numeric>
#include <ranges>
#include <set>
#include <sstream>

namespace ranges = std::ranges;

struct ingredientlist_t {
  std::set<std::string> ingredients;
  std::set<std::string> allergens;
};
using foods_t = std::vector<ingredientlist_t>;

auto read_foods(const auto& lines) {
  auto foods = foods_t (lines.size());
  ranges::transform(lines, foods.begin(), [](const auto& line){
    auto list = ingredientlist_t {};
    auto ss = std::stringstream (line);
    std::string ingredients;
    while(std::getline(ss, ingredients, '(').good()) {
      std::string ingredient;
      auto iss = std::stringstream (ingredients);
      while(std::getline(iss, ingredient, ' ').good()) {
        list.ingredients.insert(ingredient);
      }
    }
    std::string allergens = ingredients.substr(9);
    auto ass = std::stringstream (allergens);
    std::string allergen;
    while(std::getline(ass, allergen, ' ').good()) {
      list.allergens.insert(allergen.substr(0, allergen.size()-1));
    }
    list.allergens.insert(allergen.substr(0, allergen.size()-1));
    return list;
  });
  return foods;
}

auto safe_ingredients(const foods_t& foods) {
  using items_set = std::set<std::string>;
  auto allergens = std::unordered_map<std::string, items_set> {};
  auto ingredients = items_set {};
  ranges::for_each(foods, [&](const ingredientlist_t& list) {
    ranges::copy(list.ingredients, std::inserter(ingredients, ingredients.end()));
    ranges::for_each(list.allergens, [&](const auto& allergen) {
      auto ingredients = items_set {};
      ranges::copy(list.ingredients, std::inserter(ingredients, ingredients.begin()));
      if (!allergens.contains(allergen)) {
        allergens[allergen] = ingredients;
      } else {
        auto intersect = items_set {};
        ranges::set_intersection(ingredients, allergens[allergen], std::inserter(intersect, intersect.end()));
        allergens[allergen] = intersect;
      }
    });
  });
  auto safe_ingredients = std::set<std::string> {};
  ranges::for_each(ingredients, [&](const auto& ingredient){
    if (ranges::none_of(allergens, [&](const auto& allergen){
      return allergen.second.contains(ingredient);
    })) {
      safe_ingredients.insert(ingredient);
    }
  });
  return safe_ingredients;
}

auto all_allergens(const foods_t& foods) {
  std::set<std::string> allergens;
  ranges::for_each(foods, [&](const ingredientlist_t& food){
    ranges::copy(food.allergens, std::inserter(allergens, allergens.end()));
  });
  return allergens;
}

auto solve_allergens(const foods_t& foods) {
  auto solution = std::map<std::string, std::string> {};
  auto allergens_in = std::unordered_map<std::string, std::set<std::string>> {};

  // Intersect all ingredient sets of food that contains an allergen
  ranges::for_each(foods, [&allergens_in](const ingredientlist_t &food) {
    ranges::for_each(food.allergens, [&food, &allergens_in](const auto &food_allergen) {
      auto ingredients = food.ingredients;
      if (allergens_in.contains(food_allergen)) {
        auto intersect = std::set<std::string>{};
        ranges::set_intersection(ingredients,
                                 allergens_in.at(food_allergen),
                                 std::inserter(intersect, intersect.begin()));
        allergens_in.at(food_allergen) = intersect;
      } else {
        allergens_in[food_allergen] = ingredients;
      }
    });
  });

  for (;;) {
    // Find allergens for which only 1 option remains
    auto solved = std::vector<std::string> {};
    ranges::for_each(allergens_in, [&](auto &all_in) {
      auto ingredient_opts = all_in.second.size();
      if (ingredient_opts == 1) {
        solved.push_back(all_in.first);
        solution[all_in.first] = *all_in.second.cbegin();
      }
    });
    if (solved.empty())
      break;

    // Remove that option from the allergen list and from food options
    ranges::for_each(solved, [&](const auto& allergen){
      const auto& ingredient = solution[allergen];
      allergens_in.erase(allergen);
      ranges::for_each(allergens_in, [&](auto& ain) {
        auto& ingredients = ain.second;
        ingredients.erase(ingredient);
      });
    });
    if (allergens_in.empty())
      break;
  }

  return solution;
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

  auto foods = read_foods(tokens);
  auto allergens = all_allergens(foods);
  auto safe = safe_ingredients(foods);

  // Part 1: count occurence of safe ingredients in all food
  auto occurence = std::vector<size_t> (foods.size());
  ranges::transform(foods, occurence.begin(), [&safe](const auto& food){
    return ranges::count_if(food.ingredients, [&safe](const auto& ingredient){
      return safe.contains(ingredient);
    });
  });
  auto a1 = std::accumulate(occurence.cbegin(), occurence.cend(), size_t{0});
  std::cout << "Part 1: " << a1 << "\n";

  // Part 2: solve the allergen-ingredient map
  std::string a2;
  auto allergen_food = solve_allergens(foods);
  ranges::for_each(allergen_food, [&](const auto& af){
    a2.append(af.second);
    a2.push_back(',');
  });
  a2.pop_back();
  std::cout << "Part 2: " << a2 << "\n";
  std::cout << a1 << "\n" << a2 << "\n";
}