#ifndef LIB_XCORE_UTILS_COMMAND_PARSER_HPP
#define LIB_XCORE_UTILS_COMMAND_PARSER_HPP

#include "internal/macros.hpp"

#include <cstring>
#include <cstddef>

LIB_XCORE_BEGIN_NAMESPACE

/**
   * Simple whitespace-delimited C-string command parser.
   *
   * Copies the input into an internal fixed-size buffer, then tokenizes it
   * in-place. No heap allocation. Safe for embedded targets.
   *
   * Usage:
   *   command_parser_t<> parser;
   *   if (parser.parse("move 10 20")) {
   *       parser.command();   // "move"
   *       parser.argv[1];     // "10"
   *       parser.argc();      // 3
   *   }
   *
   * @tparam BufferSize  Maximum input length (including null terminator).
   * @tparam MaxArgs     Maximum number of tokens (command + arguments).
   */
template<size_t BufferSize = 64, size_t MaxArgs = 8>
class command_parser_t {
  char        buffer_[BufferSize];
  const char *argv_data_[MaxArgs];
  size_t      argc_;

public:
  /** Subscriptable view over the parsed tokens. Returns nullptr if out of range. */
  struct {
    const char **data_;
    const size_t *argc_;

    [[nodiscard]] const char *operator[](size_t i) const {
      return (i < *argc_) ? data_[i] : nullptr;
    }
  } argv;

  command_parser_t() : buffer_{}, argv_data_{}, argc_(0), argv{argv_data_, &argc_} {}

  /**
     * Parse a C-string command. Tokens are separated by whitespace (' ' or '\t').
     * Returns true if at least one token was found.
     * Returns false if input is null or exceeds BufferSize.
     */
  bool parse(const char *input) {
    argc_ = 0;

    if (!input) return false;

    const size_t len = strlen(input);
    if (len >= BufferSize) return false;

    memcpy(buffer_, input, len + 1);

    char *p = buffer_;
    while (*p && argc_ < MaxArgs) {
      // Skip leading whitespace
      while (*p == ' ' || *p == '\t') ++p;
      if (!*p) break;

      argv_data_[argc_++] = p;

      // Advance to end of token
      while (*p && *p != ' ' && *p != '\t') ++p;
      if (*p) *p++ = '\0';
    }

    return argc_ > 0;
  }

  /** Number of tokens (command + arguments). */
  [[nodiscard]] size_t argc() const { return argc_; }

  /** First token (the command name). Equivalent to argv[0]. */
  [[nodiscard]] const char *command() const { return argv[0]; }

  /** Returns true if the command matches the given string. */
  [[nodiscard]] bool is(const char *cmd) const {
    const char *c = command();
    return c && cmd && strcmp(c, cmd) == 0;
  }
};

LIB_XCORE_END_NAMESPACE

#endif  //LIB_XCORE_UTILS_COMMAND_PARSER_HPP