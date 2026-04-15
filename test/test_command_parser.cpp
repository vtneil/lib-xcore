#include "lib_xcore"
#include <iostream>

static int pass_count = 0;
static int fail_count = 0;

static void check(const bool condition, const char *label) {
  if (condition) {
    std::cout << "  [PASS] " << label << "\n";
    ++pass_count;
  } else {
    std::cout << "  [FAIL] " << label << "\n";
    ++fail_count;
  }
}

static void section(const char *title) {
  std::cout << "\n"
            << title << "\n";
}

int main() {
  xcore::command_parser_t<> parser;

  // -------------------------------------------------------------------------
  section("Basic parsing");
  // -------------------------------------------------------------------------
  {
    parser.parse("move 10 20");
    check(parser.argc() == 3, "argc == 3");
    check(strcmp(parser.command(), "move") == 0, "command == \"move\"");
    check(strcmp(parser.argv(1), "10") == 0, "argv(1) == \"10\"");
    check(strcmp(parser.argv(2), "20") == 0, "argv(2) == \"20\"");
    check(parser.argv(3) == nullptr, "argv(3) == nullptr (out of range)");
    check(parser.is("move"), "is(\"move\") == true");
    check(!parser.is("stop"), "is(\"stop\") == false");
  }

  // -------------------------------------------------------------------------
  section("operator[]");
  // -------------------------------------------------------------------------
  {
    parser.parse("go 5");
    check(strcmp(parser[0], "go") == 0, "parser[0] == \"go\"");
    check(strcmp(parser[1], "5") == 0, "parser[1] == \"5\"");
    check(parser[2] == nullptr, "parser[2] == nullptr");
  }

  // -------------------------------------------------------------------------
  section("Command only (no arguments)");
  // -------------------------------------------------------------------------
  {
    parser.parse("reset");
    check(parser.argc() == 1, "argc == 1");
    check(strcmp(parser.command(), "reset") == 0, "command == \"reset\"");
    check(parser.argv(1) == nullptr, "argv(1) == nullptr");
  }

  // -------------------------------------------------------------------------
  section("Leading, trailing, and extra whitespace");
  // -------------------------------------------------------------------------
  {
    parser.parse("  set  x  42  ");
    check(parser.argc() == 3, "argc == 3");
    check(strcmp(parser.command(), "set") == 0, "command == \"set\"");
    check(strcmp(parser.argv(1), "x") == 0, "argv(1) == \"x\"");
    check(strcmp(parser.argv(2), "42") == 0, "argv(2) == \"42\"");
  }

  // -------------------------------------------------------------------------
  section("Tab separation");
  // -------------------------------------------------------------------------
  {
    parser.parse("read\t0xFF\t4");
    check(parser.argc() == 3, "argc == 3");
    check(strcmp(parser.command(), "read") == 0, "command == \"read\"");
    check(strcmp(parser.argv(1), "0xFF") == 0, "argv(1) == \"0xFF\"");
    check(strcmp(parser.argv(2), "4") == 0, "argv(2) == \"4\"");
  }

  // -------------------------------------------------------------------------
  section("Null and empty input");
  // -------------------------------------------------------------------------
  {
    check(!parser.parse(nullptr), "parse(nullptr) == false");
    check(!parser.parse(""), "parse(\"\") == false");
    check(parser.argc() == 0, "argc == 0 after empty parse");
    check(parser.command() == nullptr, "command() == nullptr after empty parse");
  }

  // -------------------------------------------------------------------------
  section("Input too long");
  // -------------------------------------------------------------------------
  {
    // Default BufferSize is 64; a 64-char string leaves no room for '\0'
    const char long_input[65] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    check(!parser.parse(long_input), "parse(input >= BufferSize) == false");
  }

  // -------------------------------------------------------------------------
  section("MaxArgs clamping");
  // -------------------------------------------------------------------------
  {
    // parser with MaxArgs=3 should stop after 3 tokens
    xcore::command_parser_t<64, 3> small;
    small.parse("a b c d e");
    check(small.argc() == 3, "argc clamped to MaxArgs (3)");
    check(strcmp(small.argv(2), "c") == 0, "argv(2) == \"c\"");
    check(small.argv(3) == nullptr, "argv(3) == nullptr");
  }

  // -------------------------------------------------------------------------
  section("Re-parse overwrites previous result");
  // -------------------------------------------------------------------------
  {
    parser.parse("first 1 2");
    parser.parse("second 99");
    check(parser.argc() == 2, "argc == 2 after re-parse");
    check(strcmp(parser.command(), "second") == 0, "command == \"second\"");
    check(strcmp(parser.argv(1), "99") == 0, "argv(1) == \"99\"");
    check(parser.argv(2) == nullptr, "argv(2) == nullptr");
  }

  // -------------------------------------------------------------------------
  std::cout << "\n"
            << pass_count << " passed, "
            << fail_count << " failed.\n";

  return fail_count == 0 ? 0 : 1;
}