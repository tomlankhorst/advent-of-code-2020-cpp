#pragma once

#include <algorithm>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>
#include <ostream>

namespace handheld {

enum class ins_e {
  nop, jmp, acc,
};

static auto ins_strs = std::unordered_map<std::string_view, ins_e>{
    {"nop", ins_e::nop},
    {"jmp", ins_e::jmp},
    {"acc", ins_e::acc},
};

ins_e parse_ins(std::string_view s) {
  return ins_strs.at(s);
}

std::string_view ins_str(ins_e e) {
  return std::ranges::find_if(ins_strs, [&](const auto &pair) {
    return pair.second == e;
  })->first;
}

struct instr_t {
  ins_e ins;
  int val;
};
using program_t = std::vector<instr_t>;

auto read_program(const std::vector<std::string>& tokens) {
  auto p = program_t(tokens.size());

  std::ranges::transform(tokens, p.begin(), [](const auto& l) -> instr_t {
    return {
        parse_ins(l.substr(0, 3)),
        std::stoi(l.substr(4))
    };
  });

  return p;
}

struct machine_t {
  enum class exit_e { unknown = 0, normal, loop };
  exit_e exit = exit_e::unknown;
  int pc = 0;
  int r0 = 0;
};

machine_t execute(const program_t &program, machine_t m = {}) {
  auto visited = std::vector<bool>(program.size());

  for (;;) {
    if (m.pc >= program.size()) {
      m.exit = machine_t::exit_e::normal;
      break;
    }

    if (visited[m.pc]) {
      m.exit = machine_t::exit_e::loop;
      break;
    }

    visited[m.pc] = true;

    auto instr = program[m.pc];

    switch (instr.ins) {
      case ins_e::nop:break;
      case ins_e::jmp:m.pc += instr.val;
        continue;
      case ins_e::acc:m.r0 += instr.val;
        break;
    }
    m.pc++;
  }

  return m;
};

}

std::ostream& operator<<(std::ostream& os, const handheld::machine_t::exit_e& e) {
  using e_e = handheld::machine_t::exit_e;
  switch (e) {
    case e_e::unknown: os << "unknown"; break;
    case e_e::loop: os << "loop"; break;
    case e_e::normal: os << "normal"; break;
  }
  return os;
}
std::ostream& operator<<(std::ostream& os, const handheld::machine_t& m) {
  os << "PC: " << m.pc << ", R0: " << m.r0 << ", exit " << m.exit;
  return os;
}
