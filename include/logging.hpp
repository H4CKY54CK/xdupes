#pragma once

#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <unistd.h>


namespace logging {

  // Logger for more personalized logging
  struct Logger {
    std::string name;
    std::size_t loglevel;
    std::map<std::size_t, std::string> level_names;
    std::map<std::size_t, std::string> level_colors;

    explicit Logger(std::string name, std::size_t loglevel);
    explicit Logger(std::string name);

    // Get and set level for an instance of a logger
    auto get_level() -> std::size_t;
    void set_level(std::size_t level);

    // Convenience methods
    void log(std::size_t level, const std::string& msg);
    void fatal(const std::string& msg);
    void critical(const std::string& msg);
    void error(const std::string& msg);
    void warning(const std::string& msg);
    void warn(const std::string& msg);
    void info(const std::string& msg);
    void debug(const std::string& msg);
  };

  // Global convenience functions
  extern std::map<std::string, Logger> _logger_registry;

  Logger& get_logger(const std::string& name);

  // Get and set level for global logger
  std::size_t get_level();
  void set_level(std::size_t level);

  // Convenience methods
  void log(std::size_t level, const std::string& msg);
  void fatal(const std::string& msg);
  void critical(const std::string& msg);
  void error(const std::string& msg);
  void warning(const std::string& msg);
  void warn(const std::string& msg);
  void info(const std::string& msg);
  void debug(const std::string& msg);
};
