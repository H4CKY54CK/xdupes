#include "parser.hpp"


// Set up logger for this file.
logging::Logger& parsing::logger = logging::get_logger("parser");


void parsing::set_logging_level(std::size_t level) {
  logger.set_level(level);
}


// Argument implementation
parsing::Argument::Argument(const std::initializer_list<std::string>& values) : _option_strings(values) {
  // Verify they have leading hyphens.
  if (_option_strings.size() > 1) {
    for (const auto& flag : _option_strings) {
      if (flag.substr(0, 1) != "-") {
        logger.error("'Argument' objects containing more than 1 option string must ensure they all have a leading hyphen.");
        std::quick_exit(1);
      }
    }
    _argtype = "optional";
  }
  else if (_option_strings.size() == 1) {
    if (_option_strings.at(0).substr(0, 1) == "-") {
      _argtype = "optional";
    }
    else {
      _argtype = "positional";
      _required = true;
    }
  }
  _dest = *(std::max_element(values.begin(), values.end(), [](const auto& a, const auto& b) { return a.size() < b.size();}));
  _dest = _dest.substr(_dest.find_first_not_of("-", 0, 2));
  logger.debug("argument with option strings {" + repr_join(values, ", ") + "} set destination as " + repr(_dest));
}

auto parsing::Argument::dest(const std::string& value) -> Argument& {
  if (_argtype == "positional") {
    logger.error("'dest' already provided for positional argument");
    std::quick_exit(1);
  }
  _dest = value;
  return *this;
}

auto parsing::Argument::action(const std::string& value) -> Argument& {
  if (value == "store_true") {
    _min_nargs = 0;
    _max_nargs = 0;
    if (_default.size() == 0) {
      _default = "false";
    }
    if (_const.size() == 0) {
      _const = "true";
    }
  }
  else if (value == "store_false") {
    _min_nargs = 0;
    _max_nargs = 0;
    if (_default.size() == 0) {
      _default = "true";
    }
    if (_const.size() == 0) {
      _const = "false";
    }
  }
  else if (value == "store_const") {
    _min_nargs = 0;
    _max_nargs = 0;
  }
  // else if (value == "append" or value == "append_const" or value == "store" or value == "count") {
  //   _action = value;
  // }
  else {
    logger.error("unrecognized action: " + value);
    std::quick_exit(1);
  }
  _action = value;
  return *this;
}

auto parsing::Argument::nargs(const std::string& value) -> Argument& {
  if (value == "?") {
    _min_nargs = 0;
    _max_nargs = 1;
  }
  else if (value == "*") {
    _min_nargs = 0;
    _max_nargs = -1;
  }
  else if (value == "+") {
    _min_nargs = 1;
    _max_nargs = -1;
  }
  return *this;
}

auto parsing::Argument::nargs(std::size_t value) -> Argument& {
  _min_nargs = 1;
  _max_nargs = value;
  return *this;
}

auto parsing::Argument::choices(const std::initializer_list<std::string>& values) -> Argument& {
  _choices = values;
  return *this;
}

auto parsing::Argument::const_value(const std::string& value) -> parsing::Argument& {
  _const = value;
  return *this;
}

auto parsing::Argument::default_value(const std::string& value) -> parsing::Argument& {
  _default = value;
  return *this;
}

auto parsing::Argument::required(bool value) -> Argument& {
  _required = value;
  return *this;
}

auto parsing::Argument::help(const std::string& value) -> Argument& {
  _help = value;
  return *this;
}

auto parsing::Argument::metavar(const std::string& value) -> Argument& {
  _metavar = value;
  return *this;
}



// Result implementation
// This is literal trash, wrote in a haste. Get it the fuck fixed.

parsing::Result::operator bool() const {
  if (values.size() == 0) {
    return false;
  }
  if (values.at(0) == "true") {
    return true;
  }
  else if (values.at(0) == "false") {
    return false;
  }
  throw std::invalid_argument("not a boolean");
}

parsing::Result::operator int() const {
  if (values.size() == 0) {
    return 0;
  }
  if (!is_number(values.at(0))) {
    throw std::invalid_argument("not an integer");
  }
  return std::stoi(values.at(0));
}

parsing::Result::operator std::string() const {
  if (values.size() == 0) {
    return "";
  }
  return values.at(0);
}

parsing::Result::operator std::size_t() const {
  if (values.size() == 0) {
    return 0;
  }
  if (!is_number(values.at(0))) {
    throw std::invalid_argument("not an integer");
  }
  return std::stoul(values.at(0));
}

parsing::Result::operator std::vector<std::string>() const {
  if (values.size() == 0) {
    return {};
  }
  return values;
}

parsing::Result::operator std::vector<int>() const {
  if (values.size() == 0) {
    return {};
  }
  std::vector<int> vec;
  for (const auto& item : values) {
    if (!is_number(item)) {
      throw std::invalid_argument("not all items were integers");
    }
    vec.emplace_back(std::stoul(item));
  }
  return vec;
}

auto parsing::Result::as_bool() -> bool {
  return bool(*this);
}
auto parsing::Result::as_int() -> int {
  return int(*this);
}
auto parsing::Result::as_char() -> char {
  return std::string(*this).c_str()[0];
}
auto parsing::Result::as_size_t() -> std::size_t {
  return std::size_t(*this);
}
auto parsing::Result::as_string() -> std::string {
  return std::string(*this);
}
auto parsing::Result::as_strings() -> std::vector<std::string> {
  return std::vector<std::string>(*this);
}
auto parsing::Result::as_ints() -> std::vector<int> {
  return std::vector<int>(*this);
}


// ArgumentParser implementation
parsing::ArgumentParser::ArgumentParser(std::string name) : name(std::move(name)) {}

void parsing::ArgumentParser::add_argument(const Argument& argument) {
  if (argument._argtype == "optional") {
    for (const auto& flag : argument._option_strings) {
      if (optionals.count(flag) > 0) {
        logger.error("cannot use the same flag multiple times: " + repr(flag));
        std::quick_exit(1);
      }
      optionals.emplace(flag, argument);
    }
  }
  else if (argument._argtype == "positional") {
    for (const auto& arg : positionals) {
      if (arg._dest == argument._dest) {
        logger.error("cannot use the same dest with multiple positional arguments: " + repr(arg._dest));
        std::quick_exit(1);
      }
    }
    positionals.emplace_back(argument);
  }
  else {
    logger.error("unrecognized argument type: " + repr(argument._argtype));
    std::quick_exit(1);
  }
}

auto parsing::ArgumentParser::parse_args(int argc, char** argv) -> std::map<std::string, Result> {
  std::map<std::string, Result> result;

  std::vector<std::string> remaining;

  std::string value;

  std::string arg;
  bool keep_parsing_options = true;

  for (int ix = 0; ix < argc; ++ix) {
    arg = argv[ix];

    // Whether we have encountered a "--" or not
    if (keep_parsing_options) {

      // Looks like an option
      if (arg.substr(0, 1) == "-") {

        // Stop parsing options
        if (arg == "--") {
          keep_parsing_options = false;
          continue;
        }

        // Unrecognized arguments
        if (optionals.count(arg) == 0) {
          logger.error("unrecognized option argument: " + repr(arg));
          std::quick_exit(1);
        }

        Argument& option = optionals.at(arg);

        // Have we seen the argument before?
        if (result.count(option._dest) > 0) {
          // If so, then these are the ones that aren't allowed to show up again
          if ((option._action == "store_true") or (option._action == "store_const") or (option._action == "help") or
              (option._action == "store_false") or (option._action == "store") or (option._action == "version")) {
            logger.error("option argument already provided: " + repr(arg));
            std::quick_exit(1);
          }
        }

        // Assign true, held by _const
        if (option._action == "store_true") {
          logger.info("store_true");
          result[option._dest].values.emplace_back(option._const);
          continue;
        }

        // Assign false, held by _const
        if (option._action == "store_false") {
          logger.info("store_false");
          result[option._dest].values.emplace_back(option._const);
          continue;
        }

        // Assign _const, held by _const
        if (option._action == "store_const") {
          logger.info("store_const");
          // result[option._dest].emplace_bacl(option._const);
          result[option._dest].values.emplace_back(option._const);
          continue;
        }

        // Append _const, held by _const
        if (option._action == "append_const") {
          logger.info("append_const");
          result[option._dest].values.emplace_back(option._const);
          continue;
        }

        // Increment
        if (option._action == "count") {
          logger.info("count");
          result[option._dest].values.emplace_back("1");
          continue;
        }

        // Appends N amount of items to the result, as a vector. Cannot implement at this time.
        // else if (option._action == "append") {
        //   result[option._dest].emplace_back();
        // }

        // Appends N amount of items to the result, individually.
        if (option._action == "extend") {
          while ((ix <= argc) and (remaining.size() < option._max_nargs)) {
            ++ix;
            value = argv[ix];
            // It's possible we're parsing an unknown amount of args, and so it's possible for an option to be valid
            // here, as well as be invalid here. However, I don't see how a valid option could actually get parsed.
            // Can it be decremented?
            if (value.substr(0, 1) == "-") {
              break;
              // --ix;
            }
            result[option._dest].values.emplace_back(value);
          }
          if (remaining.size() < option._min_nargs) {
            logger.error(repr(arg) + " expects at least " + std::to_string(option._min_nargs) + " values, but got " + std::to_string(remaining.size()));
            std::quick_exit(1);
          }
          continue;
        }

        // Intended for a single value
        if (option._action == "store") {
          if (ix >= (argc - 1)) {
            logger.error(repr(arg) + " expects exactly 1 value, but got 0");
            std::quick_exit(1);
          }
          ++ix;
          value = argv[ix];
          if (value.substr(0, 1) == "-") {
            logger.error(repr(arg) + " expects exactly 1 value, but encountered an ambiguous argument: " + repr(arg));
            std::quick_exit(1);
          }
          result[option._dest].values.emplace_back(value);
          continue;
        }

        // Unrecognized action. This was checked during creation of the Argument, but check again anyway.
        logger.error("Unrecognized action: " + repr(option._action));
        std::quick_exit(1);
      }
    }

    // Positional
    remaining.emplace_back(arg);
  }

  std::vector<std::size_t> layout;
  std::size_t minimum = 0;
  std::size_t maximum = 0;

  for (const auto& position : positionals) {
    layout.emplace_back(position._min_nargs);
    minimum += position._min_nargs;
    maximum += position._max_nargs;
  }

  if (remaining.size() < minimum) {
    logger.error("not enough positional arguments: expecting at least " + std::to_string(minimum) + ", but got " + std::to_string(remaining.size()));
    std::quick_exit(1);
  }
  else if (remaining.size() > maximum) {
    logger.error("too many positional arguments: expecting at most " + std::to_string(maximum) + ", but got " + std::to_string(remaining.size()));
    std::quick_exit(1);
  }

  while (positionals.size() > 0) {
    auto position = positionals.back();
    positionals.erase(positionals.end());
    while (result[position._dest].values.size() < position._min_nargs) {
      auto value = remaining.back();
      remaining.erase(remaining.end());
      result[position._dest].values.insert(result[position._dest].values.begin(), value);
    }
    if (positionals.size() == 0) {
      while (remaining.size()) {
        auto value = remaining.back();
        remaining.erase(remaining.end());
        result[position._dest].values.insert(result[position._dest].values.begin(), value);
      }
    }
  }

  for (const auto& [key, value] : optionals) {
    if (result.count(value._dest) == 0) {
      if (value._default.size() > 0) {
        result[value._dest].values.emplace_back(value._default);
      }
    }
  }
  // for (const auto& item : positionals) {
  //   if (result.count(item._dest) == 0) {
  //     result.emplace(item._dest, Result{});
  //   }
  // }

  return result;
}
