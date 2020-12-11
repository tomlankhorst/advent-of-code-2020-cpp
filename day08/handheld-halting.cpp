#include "handheld.hpp"

#include "day05/arg_input.hpp"
#include "day05/tokenize.hpp"

auto main(int argc, char* argv[]) -> int {
  using namespace handheld;
  
  auto file = get_input(argc, argv);

  if (std::holds_alternative<int>(file))
    return std::get<int>(file);

  auto& input = std::get<std::ifstream>(file);

  auto program = read_program(tokenize(input));

  std::cout << "Part 1: " << execute(program) << "\n";

  using from_to_t = std::pair<ins_e, ins_e>;
  for (const auto& [from, to] : { from_to_t{ins_e::jmp, ins_e::nop}, {ins_e::nop, ins_e::jmp} }) {
    for (size_t i = 0; i < program.size(); i++) {
      auto copy = program;
      if (copy[i].ins == from) {
        copy[i].ins = to;
        auto machine = execute(copy);
        if (machine.exit == machine_t::exit_e::normal)
          std::cout << "Part 2: Changed instruction " << i << " " << ins_str(from) << " to " << ins_str(to) << "; " << machine << "\n";
      }
    }
  }
}