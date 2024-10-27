#pragma once

#include <chrono>
#include <csignal>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


void restore_terminal(int s);


// Just a blob of data
struct TimeStats {
  std::size_t proc_start;
  std::size_t parsing;
  std::size_t filesystem;
  std::size_t hashing;
  std::size_t overall;
  std::size_t proc_stop;
};


// Some timing utilities
auto now() -> std::size_t;

auto is_number(const std::string& value) -> bool;

// bad uwu
auto repr(const std::string& value) -> std::string;
auto repr(std::size_t value) -> std::string;
auto join(const std::vector<std::string>& vec, const std::string& sep) -> std::string;
auto repr_join(const std::vector<std::string>& vec, const std::string& sep) -> std::string;

// File checks
auto file_is_unreadable(const std::filesystem::path& path) -> bool;

// Ported some of my Python utils from hackytools.
auto ftime_ns(std::size_t nanoseconds) -> std::string;
auto ftime(std::size_t seconds) -> std::string;
auto fsize(std::size_t bytes, bool use_binary_prefix) -> std::string;

// Hexdigest for xxhash hashes
auto hexdigest(std::uint64_t hash) -> std::string;
