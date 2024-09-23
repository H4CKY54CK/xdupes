#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <unistd.h>


namespace logging {

  class Handler {

  };

  // Logger for more personalized logging
  struct Logger {
    std::string name;
    std::size_t loglevel = 30;

    bool use_color;
    std::map<std::size_t, std::string> level_names = {
      {10, "debug"},
      {20, "info"},
      {30, "warn"},
      {40, "error"},
      {50, "critical"},
    };
    std::map<std::size_t, std::string> level_colors = {
      {0, "\x1b[00m"},
      {10, "\x1b[36m"},
      {20, "\x1b[32m"},
      {30, "\x1b[33m"},
      {40, "\x1b[31m"},
      {50, "\x1b[41m"},
    };


    explicit Logger();
    explicit Logger(std::string value1);
    explicit Logger(std::string value1, std::size_t value2);

    // Get loglevel (public)
    void set_level(std::size_t level);
    auto get_level() -> std::size_t;

    // Get name (public)
    auto get_name() -> std::string;
    void set_name(const std::string& value);

    auto get_color(std::size_t level) -> std::string;
    auto get_level_name(std::size_t level) -> std::string;

    // Convenience methods
    void log(std::size_t level, const std::string& msg, const std::string& tag);
    void log(std::size_t level, const std::string& msg);
    void fatal(const std::string& msg, const std::string& tag);
    void fatal(const std::string& msg);
    void critical(const std::string& msg, const std::string& tag);
    void critical(const std::string& msg);
    void error(const std::string& msg, const std::string& tag);
    void error(const std::string& msg);
    void warning(const std::string& msg, const std::string& tag);
    void warning(const std::string& msg);
    void warn(const std::string& msg, const std::string& tag);
    void warn(const std::string& msg);
    void info(const std::string& msg, const std::string& tag);
    void info(const std::string& msg);
    void debug(const std::string& msg, const std::string& tag);
    void debug(const std::string& msg);
  };

  // Global convenience functions
  extern std::map<std::string, Logger> _logger_registry;
  extern logging::Logger& _logger;

  Logger& get_logger(const std::string& name);

  void set_level(std::size_t level);
  std::size_t get_level();

  // Only helpful for global logger
  void set_name(const std::string&);
  std::string get_name();

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
