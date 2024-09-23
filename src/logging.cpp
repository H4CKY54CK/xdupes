#include "logging.hpp"


// Local logger
logging::Logger::Logger() : Logger("root") {}
logging::Logger::Logger(std::string value1) : Logger(std::move(value1), 30) {}
logging::Logger::Logger(std::string value1, std::size_t value2) : name(std::move(value1)), loglevel(value2), use_color(isatty(2) == 0) {}

// Get loglevel (public)
void logging::Logger::set_level(std::size_t level) {
  loglevel = level;
}

auto logging::Logger::get_level() -> std::size_t {
  return loglevel;
}

// Get name (public)
auto logging::Logger::get_name() -> std::string {
  return name;
}

void logging::Logger::set_name(const std::string& value) {
  name = value;
}

auto logging::Logger::get_color(std::size_t level) -> std::string {
  if (use_color) {
    return "";
  }
  return level_colors[level];
}

auto logging::Logger::get_level_name(std::size_t level) -> std::string {
  return level_names[level];
}



// Emit local logger messages
void logging::Logger::log(std::size_t level, const std::string& msg, const std::string& name) {
  std::string tag = name;
  if (name == "") {
    tag = this->name;
  }
  if (loglevel <= level) {
    std::clog << get_color(level) << "[" << tag << " " << get_level_name(level) << "]" << get_color(0) << ": " << msg << "\n";
    std::clog.flush();
  }
}
void logging::Logger::log(std::size_t level, const std::string& msg) {
  log(level, msg, "");
}
void logging::Logger::fatal(const std::string& msg, const std::string& name) {
  log(50, msg, name);
}
void logging::Logger::fatal(const std::string& msg) {
  log(50, msg);
}
void logging::Logger::critical(const std::string& msg, const std::string& name) {
  log(50, msg, name);
}
void logging::Logger::critical(const std::string& msg) {
  log(50, msg);
}
void logging::Logger::error(const std::string& msg, const std::string& name) {
  log(40, msg, name);
}
void logging::Logger::error(const std::string& msg) {
  log(40, msg);
}
void logging::Logger::warning(const std::string& msg, const std::string& name) {
  log(30, msg, name);
}
void logging::Logger::warning(const std::string& msg) {
  log(30, msg);
}
void logging::Logger::warn(const std::string& msg, const std::string& name) {
  log(30, msg, name);
}
void logging::Logger::warn(const std::string& msg) {
  log(30, msg);
}
void logging::Logger::info(const std::string& msg, const std::string& name) {
  log(20, msg, name);
}
void logging::Logger::info(const std::string& msg) {
  log(20, msg);
}
void logging::Logger::debug(const std::string& msg, const std::string& name) {
  log(10, msg, name);
}
void logging::Logger::debug(const std::string& msg) {
  log(10, msg);
}




std::map<std::string, logging::Logger> logging::_logger_registry;

// Get logger by name
auto logging::get_logger(const std::string& name) -> Logger& {
  return _logger_registry.try_emplace(name, name).first->second;
}

// The "global" logger. It always exists.
logging::Logger& logging::_logger = logging::get_logger("root");

void logging::set_level(std::size_t level) {
  _logger.loglevel = level;
}
auto logging::get_level() -> std::size_t {
  return _logger.loglevel;
}

// Get and set name
void logging::set_name(const std::string& name) {
  _logger.name = name;
}
auto logging::get_name() -> std::string {
  return _logger.name;
}

void logging::fatal(const std::string& msg) {
  _logger.log(50, msg);
}
void logging::critical(const std::string& msg) {
  _logger.log(50, msg);
}
void logging::error(const std::string& msg) {
  _logger.log(40, msg);
}
void logging::warning(const std::string& msg) {
  _logger.log(30, msg);
}
void logging::warn(const std::string& msg) {
  _logger.log(30, msg);
}
void logging::info(const std::string& msg) {
  _logger.log(20, msg);
}
void logging::debug(const std::string& msg) {
  _logger.log(10, msg);
}


