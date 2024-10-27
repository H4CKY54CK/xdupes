#pragma once

#include <algorithm>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "logging.hpp"
#include "utils.hpp"


namespace parsing {
  enum class actions: int {store_true, store_false, store_const, store, append, append_const, extend, version, count, help};
  enum class argtypes: int {positional, optional};

  extern logging::Logger& logger;

  extern std::unordered_map<actions, std::string> actions_mapping;
  extern std::unordered_map<argtypes, std::string> argtypes_mapping;

  void set_logging_level(std::size_t level);

  // Argument declaration
  struct Argument {
    std::vector<std::string> flags_;
    std::string dest_;
    argtypes argtype_ = argtypes::optional;
    actions action_ = actions::store;
    std::string nargs_ = "@";
    std::size_t min_nargs_ = 1;
    std::size_t max_nargs_ = 1;
    std::vector<std::string> choices_ = {};
    std::string const_;
    std::string default_;
    bool required_ = false;
    std::string help_ = "";
    std::string metavar_ = "";

    Argument() = default;
    explicit Argument(const std::initializer_list<std::string>& values);

    Argument& dest(const std::string& value);
    Argument& action(actions action);
    Argument& nargs(const std::string& value);
    Argument& nargs(std::size_t value);
    Argument& choices(const std::initializer_list<std::string>& values);
    Argument& const_value(const std::string& value);
    Argument& default_value(const std::string& value);
    Argument& required(bool value);
    Argument& help(const std::string& value);
    Argument& metavar(const std::string& value);
  };


  struct ArgumentGroup {
    std::string name;
    std::vector<Argument> arguments;
    std::unordered_map<std::string, std::size_t> flags;
    ArgumentGroup(std::string name);
    void add_argument(const Argument& argument);
  };



  // Result declaration
  struct Result {
    std::vector<std::string> values;
    explicit operator bool() const;
    explicit operator int() const;
    explicit operator std::string() const;
    explicit operator std::size_t() const;
    explicit operator std::vector<std::string>() const;
    explicit operator std::vector<int>() const;

    bool as_bool();
    int as_int();
    char as_char();
    std::size_t as_size_t();
    std::string as_string();
    std::vector<std::string> as_strings();
    std::vector<int> as_ints();
  };



  // ArgumentParser declaration
  struct ArgumentParser {
    std::string name = "parser";
    std::string usage;
    std::string description;
    std::string version;
    std::vector<ArgumentGroup> groups;

    explicit ArgumentParser(std::string name);

    void add_argument(const Argument& argument);

    auto parse_known_args(int argc, char** argv) -> std::pair<std::unordered_map<std::string, Result>, std::vector<std::string>>;
    auto parse_args(int argc, char** argv) -> std::unordered_map<std::string, Result>;
    auto parse_known_intermixed_args(int argc, char** argv) -> std::unordered_map<std::string, Result>;
    auto parse_intermixed_args(int argc, char** argv) -> std::unordered_map<std::string, Result>;
  };
};
