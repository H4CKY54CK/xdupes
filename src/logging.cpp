#include "logging.hpp"


// Namespace structs and classes

// Logger contstructors
logging::Logger::Logger(std::string name, std::size_t loglevel) : name(std::move(name)), loglevel(loglevel) {
  level_names = {{10, "debug"},{20, "info"},{30, "warn"},{40, "error"},{50, "critical"}};
  level_colors = {{0, "\x1b[00m"},{10, "\x1b[36m"},{20, "\x1b[32m"},{30, "\x1b[33m"},{40, "\x1b[31m"},{50, "\x1b[41m"}};
}
logging::Logger::Logger(std::string name) : Logger(std::move(name), 30) {}

// Get level for an instance of a logger
auto logging::Logger::get_level() -> std::size_t {
  return loglevel;
}

// Set level for an instance of a logger
void logging::Logger::set_level(std::size_t level) {
  loglevel = level;
}

// Get level color for an instance of a logger
auto logging::Logger::get_level_color(std::size_t level) -> std::string {
  return level_colors[level];
}

// Get level name for an instance of a logger
auto logging::Logger::get_level_name(std::size_t level) -> std::string {
  return level_names[level];
}

// Emit local logger messages
void logging::Logger::log(std::size_t level, const std::string& msg) {
  if (level >= loglevel) {
    std::cerr << get_level_color(level) << "[" << name << " "
              << get_level_name(level) << "]" << get_level_color(0)
              << ": " << msg << "\n";
  }
}
void logging::Logger::fatal(const std::string& msg) {
  log(50, msg);
}
void logging::Logger::critical(const std::string& msg) {
  log(50, msg);
}
void logging::Logger::error(const std::string& msg) {
  log(40, msg);
}
void logging::Logger::warning(const std::string& msg) {
  log(30, msg);
}
void logging::Logger::warn(const std::string& msg) {
  log(30, msg);
}
void logging::Logger::info(const std::string& msg) {
  log(20, msg);
}
void logging::Logger::debug(const std::string& msg) {
  log(10, msg);
}



// Namespace functions and members

std::map<std::string, logging::Logger> logging::_logger_registry;

// Get logger by name
auto logging::get_logger(const std::string& name) -> Logger& {
  return _logger_registry.try_emplace(name, name).first->second;
}

// Get level for global logger
auto logging::get_level() -> std::size_t {
  return get_logger("root").loglevel;
}

// Set level for global logger
void logging::set_level(std::size_t level) {
  get_logger("root").loglevel = level;
}

// Convenience functions for the namespace
void logging::log(std::size_t level, const std::string& msg) {
  get_logger("root").log(level, msg);
}
void logging::fatal(const std::string& msg) {
  log(50, msg);
}
void logging::critical(const std::string& msg) {
  log(50, msg);
}
void logging::error(const std::string& msg) {
  log(40, msg);
}
void logging::warning(const std::string& msg) {
  log(30, msg);
}
void logging::warn(const std::string& msg) {
  log(30, msg);
}
void logging::info(const std::string& msg) {
  log(20, msg);
}
void logging::debug(const std::string& msg) {
  log(10, msg);
}


