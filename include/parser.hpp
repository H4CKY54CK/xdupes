#pragma once

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "logging.hpp"
#include "utils.hpp"


namespace parsing {
  extern logging::Logger& logger;

  void set_logging_level(std::size_t level);

  // Argument declaration
  struct Argument {
    std::vector<std::string> _option_strings;
    std::string _dest;
    std::string _argtype;
    std::string _action = "store";
    std::string _nargs = "@";
    std::size_t _min_nargs = 1;
    std::size_t _max_nargs = 1;
    std::vector<std::string> _choices = {};
    std::string _const;
    std::string _default;
    bool _required = false;
    std::string _help = "";
    std::string _metavar = "";

    explicit Argument(const std::initializer_list<std::string>& values);

    Argument& dest(const std::string& value);
    Argument& action(const std::string& value);
    Argument& nargs(const std::string& value);
    Argument& nargs(std::size_t value);
    Argument& choices(const std::initializer_list<std::string>& values);
    Argument& const_value(const std::string& value);
    Argument& default_value(const std::string& value);
    Argument& required(bool value);
    Argument& help(const std::string& value);
    Argument& metavar(const std::string& value);
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


  struct ParsedArguments {
    std::map<std::string, Result> data;
    std::map<std::string, bool> valid;

    void add_valid_dest(const std::string& value);

    void add_arg(const std::string& value, const Result& result);
  };

  // ArgumentParser declaration
  struct ArgumentParser {
    std::string name = "parser";
    std::map<std::string, Argument> optionals;
    std::vector<Argument> positionals;

    explicit ArgumentParser(std::string name);

    void add_argument(const Argument& argument);

    auto parse_args(int argc, char** argv) -> std::map<std::string, Result>;
    auto parse_known_args(int argc, char** argv) -> std::map<std::string, Result>;
    auto parse_intermixed_args(int argc, char** argv) -> std::map<std::string, Result>;
    auto parse_known_intermixed_args(int argc, char** argv) -> std::map<std::string, Result>;
  };
};
