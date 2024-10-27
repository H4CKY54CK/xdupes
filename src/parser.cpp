#include "parser.hpp"



std::unordered_map<parsing::actions, std::string> parsing::actions_mapping = {
  {actions::store_true, "store_true"},
  {actions::store_false, "store_false"},
  {actions::store_const, "store_const"},
  {actions::store, "store"},
  {actions::append, "append"},
  {actions::append_const, "append_const"},
  {actions::extend, "extend"},
  {actions::version, "version"},
  {actions::count, "count"},
  {actions::help, "help"},
};

std::unordered_map<parsing::argtypes, std::string> parsing::argtypes_mapping = {
  {argtypes::positional, "positional"},
  {argtypes::optional, "optional"},
};


// Set up logger for this file.
logging::Logger& parsing::logger = logging::get_logger("parser");


void parsing::set_logging_level(std::size_t level) {
  logger.set_level(level);
}


// Argument implementation
parsing::Argument::Argument(const std::initializer_list<std::string>& values) : flags_(values) {
  // Ensure at least 1 value
  if (flags_.size() == 0) {
    logger.error("Argument object must be provided at least one option flag or the dest in the constructor");
    std::quick_exit(1);
  }
  // Check all values for '-'
  for (const auto& flag : flags_) {
    // If not prefixed
    if (flag.substr(0, 1) != "-") {
      // If it's not the only value, reject it
      if (flags_.size() != 1) {
        logger.error("Argument object with multiple option flags must all be prefixed with '-'");
        std::quick_exit(1);
      }
      // Since it is the only value, it's our dest, it's a positional, and it's required (for now)
      argtype_ = argtypes::positional;
      required_ = true;
      dest_ = flag;
      break;
    }
    // While we're here, set dest to the longest value
    dest_ = (flag.size() > dest_.size()) ? flag : dest_;
  }
  // Remove at most 2 '-' from beginning
  dest_ = dest_.substr(dest_.find_first_not_of("-", 0, 2));
}

auto parsing::Argument::dest(const std::string& value) -> Argument& {
  if (argtype_ == argtypes::positional) {
    logger.error("'dest' already provided for positional argument");
    std::quick_exit(1);
  }
  dest_ = value;
  return *this;
}

auto parsing::Argument::action(actions value) -> Argument& {
  switch (value) {
    case actions::store_true: {
      min_nargs_ = 0;
      max_nargs_ = 0;
      if (default_.size() == 0) {
        default_ = "false";
      }
      if (const_.size() == 0) {
        const_ = "true";
      }
      break;
    }
    case actions::store_false: {
      min_nargs_ = 0;
      max_nargs_ = 0;
      if (default_.size() == 0) {
        default_ = "true";
      }
      if (const_.size() == 0) {
        const_ = "false";
      }
      break;
    }
    case actions::store_const:
    case actions::append_const:
    case actions::version:
    case actions::help:
    case actions::count: {
      min_nargs_ = 0;
      max_nargs_ = 0;
      break;
    }
    case actions::append:
    case actions::store: {
      break;
    }
    case actions::extend: {
      logger.error("not implemented: " + actions_mapping[value]);
      std::quick_exit(1);
    }
    default: {
      logger.error("unrecognized action: " + actions_mapping[value]);
      std::quick_exit(1);
    }
  }
  action_ = value;
  return *this;
}

auto parsing::Argument::nargs(const std::string& value) -> Argument& {
  if (value == "?") {
    min_nargs_ = 0;
    max_nargs_ = 1;
  }
  else if (value == "*") {
    min_nargs_ = 0;
    max_nargs_ = 0;
  }
  else if (value == "+") {
    min_nargs_ = 1;
    max_nargs_ = 0;
  }
  return *this;
}

auto parsing::Argument::nargs(std::size_t value) -> Argument& {
  min_nargs_ = value;
  max_nargs_ = value;
  return *this;
}

auto parsing::Argument::choices(const std::initializer_list<std::string>& values) -> Argument& {
  choices_ = values;
  return *this;
}

auto parsing::Argument::const_value(const std::string& value) -> parsing::Argument& {
  const_ = value;
  return *this;
}

auto parsing::Argument::default_value(const std::string& value) -> parsing::Argument& {
  default_ = value;
  return *this;
}

auto parsing::Argument::required(bool value) -> Argument& {
  required_ = value;
  return *this;
}

auto parsing::Argument::help(const std::string& value) -> Argument& {
  help_ = value;
  return *this;
}

auto parsing::Argument::metavar(const std::string& value) -> Argument& {
  metavar_ = value;
  return *this;
}


parsing::ArgumentGroup::ArgumentGroup(std::string name) : name(std::move(name)) {}

void parsing::ArgumentGroup::add_argument(const Argument& argument) {
  // Size of vector == index after we insert it
  for (const auto& flag : argument.flags_) {
    if (flag.substr(0, 1) == "-") {
      if (flags.count(flag) > 0) {
        logger.error("ArgumentGroup cannot contain duplicate flag: " + repr(flag));
        std::quick_exit(1);
      }
    }
  }
  for (const auto& flag : argument.flags_) {
    flags.emplace(flag, arguments.size());
  }
  arguments.emplace_back(argument);
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
parsing::ArgumentParser::ArgumentParser(std::string name) : name(std::move(name)) {
  groups.emplace_back("positional arguments");
  groups.emplace_back("options");
  groups.back().add_argument(Argument{"--help", "-h"}.action(actions::help).help("Show this menu and exit."));
}

void parsing::ArgumentParser::add_argument(const Argument& argument) {
  switch (argument.argtype_) {
    case argtypes::positional: {
      groups[0].add_argument(argument);
      break;
    }
    case argtypes::optional: {
      for (const auto& group : groups) {
        for (const auto& flag : argument.flags_) {
          if (group.flags.count(flag) > 0) {
            logger.error("ArgumentParser groups as a whole cannot contain duplicate flag: " + repr(flag));
            std::quick_exit(1);
          }
        }
      }
      groups[1].add_argument(argument);
      break;
    }
    default: {
      logger.error("unrecognized argument type: " + repr(argtypes_mapping[argument.argtype_]));
      std::quick_exit(1);
    }
  }
}

auto parsing::ArgumentParser::parse_known_args(int argc, char** argv) -> std::pair<std::unordered_map<std::string, Result>, std::vector<std::string>> {
  std::unordered_map<std::string, Result> result;
  std::vector<std::string> remaining;
  std::string value;
  std::string arg;
  std::optional<Argument> option;

  for (int ix = 0; ix < argc; ++ix) {
    arg = argv[ix];

    // Whether we have encountered a "--" or not
    if (arg.substr(0, 1) == "-") {

      // Stop parsing options
      if (arg == "--") {
        for (ix = ix + 1; ix < argc; ++ix) {
          remaining.emplace_back(argv[ix]);
        }
        break;
      }

      option.reset();

      for (const auto& group : groups) {
        if (group.flags.count(arg) > 0) {
          option = group.arguments.at(group.flags.at(arg));
          break;
        }
      }

      // Unrecognized arguments
      if (not option) {
        logger.error("unrecognized option argument: " + repr(arg));
        std::quick_exit(1);
      }

      // Have we seen the argument before?
      if (result.count(option.value().dest_) > 0) {
        switch (option.value().action_) {
          case actions::store_true:
          case actions::store_false:
          case actions::store_const:
          case actions::store:
          case actions::help:
          case actions::version: {
            logger.error("option argument already provided: " + repr(arg));
            std::quick_exit(1);
          }
          default: {
            break;
          }
        }
      }

      // Assign true, false, or const, held by const_
      switch (option.value().action_) {
        // Append is supposed to append a list, but we can't do that yet. We also haven't implemented version/help yet.
        case actions::append:
        case actions::version:
        case actions::help: {
          logger.error("append/version/help not yet implemented");
          std::quick_exit(1);
        }
        // Boolean and const flags
        case actions::store_true:
        case actions::store_false:
        case actions::store_const: {
          result[option.value().dest_].values.emplace_back(option.value().const_);
          continue;
        }
        // Intended for... idk?
        case actions::append_const: {
          result[option.value().dest_].values.emplace_back(option.value().dest_);
          continue;
        }
        // Intended for things like --verbosity
        case actions::count: {
          result[option.value().dest_].values.emplace_back("1");
          continue;
        }
        // Intended for multiple values
        case actions::extend: {
          while (ix < (argc - 1)) {
            if (result[option.value().dest_].values.size() < option.value().max_nargs_) {
              break;
            }
            ++ix;
            value = argv[ix];
            // It's possible we're parsing an unknown amount of args, and so it's possible for an option to be valid
            // here, as well as be invalid here. However, I don't see how a valid option could actually get parsed.
            // Can it be decremented?
            if (value.substr(0, 1) == "-") {
              break;
            }
            result[option.value().dest_].values.emplace_back(value);
          }
          if (result[option.value().dest_].values.size() < option.value().min_nargs_) {
            logger.error(repr(arg) + " expects at least " + repr(option.value().min_nargs_) + " values, but got " + repr(result[option.value().dest_].values.size()));
            std::quick_exit(1);
          }
          continue;
        }
        // Intended for a single value
        case actions::store: {
          ++ix;
          if (ix >= argc) {
            logger.error(repr(arg) + " expects exactly 1 value, but got 0");
            std::quick_exit(1);
          }
          value = argv[ix];
          if (value.substr(0, 1) == "-") {
            logger.error(repr(arg) + " expects exactly 1 value, but encountered an ambiguous argument: " + repr(value));
            std::quick_exit(1);
          }
          result[option.value().dest_].values.emplace_back(value);
          continue;
        }
        // Fallthrough
        default: {
          logger.error("Unrecognized action: " + repr(actions_mapping[option.value().action_]));
          std::quick_exit(1);
        }
      }
    }

    //Positional
    remaining.emplace_back(arg);
  }

  // Track min, max, and whether an exact number is needed
  // While it may seem like (exact) == (total_min == total_max), it's not. A '?' narg will have 0 min and 1 max, and a
  // '+' narg will have 1 min and 0 max. These balance out and create a false positive. What we actually want is
  // (exact) == (min == max) for every iteration of the loop.
  std::size_t minimum = 0;
  std::size_t maximum = 0;
  bool exact = true;

  // Jesus
  for (const auto& group : groups) {
    for (const auto& opt : group.arguments) {
      if (opt.argtype_ == argtypes::positional) {
        minimum += opt.min_nargs_;
        maximum += opt.max_nargs_;
        if (opt.min_nargs_ != opt.max_nargs_ and opt.max_nargs_ == 0) {
          exact = false;
        }
      }
    }
  }

  // Check for too few arguments
  if (remaining.size() < minimum) {
    std::string dest;
    std::size_t subtotal = 0;
    for (const auto& group : groups) {
      for (const auto& opt : group.arguments) {
        if (opt.argtype_ == argtypes::optional) {
          continue;
        }
        subtotal += opt.min_nargs_;
        dest = opt.dest_;
        if (subtotal > remaining.size()) {
          break;
        }
      }
    }
    logger.error("missing positional argument: " + dest);
    std::exit(1);
  }

  // If it's exact, then we have exactly the right amount
  if (exact) {
    auto it = remaining.begin();
    for (const auto& group : groups) {
      for (const auto& opt: group.arguments) {
        if (opt.argtype_ == argtypes::optional) {
          continue;
        }
        for (std::size_t ix = 0; ix < opt.min_nargs_; ++ix, ++it) {
          result[opt.dest_].values.emplace_back(*it);
        }
      }
    }

    // This replaced an error check that used to be right after the "too few args" check
    if (it != remaining.end()) {
      logger.error("unrecognized positional argument: " + remaining.at(maximum));
      std::exit(1);
    }
  }
  // And finally, the check for variable number of arguments
  else {
    // Create layout of minimum required arguments per index
    std::size_t known = 0;
    for (const auto& group : groups) {
      for (const auto& opt: group.arguments) {
        if (opt.argtype_ == argtypes::optional) {
          continue;
        }
        if (opt.min_nargs_ == 1) {
          known += opt.min_nargs_;
        }
      }
    }

    // Distribute them
    std::size_t pix = 0;
    for (const auto& group : groups) {
      for (const auto& opt: group.arguments) {
        if (opt.argtype_ == argtypes::optional) {
          continue;
        }
        // Exactly X
        if (opt.min_nargs_ == opt.max_nargs_) {
          for (std::size_t ix = 0; ix < opt.min_nargs_; ++ix) {
            result[opt.dest_].values.emplace_back(remaining.at(pix));
            ++pix;
          }
        }

        // 0 or 1
        else if (opt.min_nargs_ == 0 and opt.max_nargs_ == 1) {
          if ((remaining.size() - known) > 0) {
            result[opt.dest_].values.emplace_back(remaining.at(pix));
            ++pix;
          }
        }

        // 0 or more and 1 or more
        else if (opt.max_nargs_ == 0) {
          for (std::size_t ix = 0; ix < (remaining.size() - known); ++ix) {
            result[opt.dest_].values.emplace_back(remaining.at(pix));
            ++pix;
          }
        }
      }
    }
  }

  std::vector<std::string> other;
  return std::make_pair(result, other);
}

auto parsing::ArgumentParser::parse_args(int argc, char** argv) -> std::unordered_map<std::string, Result> {
  auto [args, options] = parse_known_args(argc, argv);
  if (not options.empty()) {
    logger.error("unrecognized options");
    std::exit(1);
  }
  return args;
}

