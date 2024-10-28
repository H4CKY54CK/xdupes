#include "utils.hpp"



// Signal handler for when progress bar gets cancelled
void restore_terminal(int s) {
  // Clear line, restore cursor position, unhide cursor.
  std::cout << "\x1b[?25h\x1b[u\x1b[2KCancelled by user..." << std::endl;
  std::quick_exit(s);
}


// TIME. IS MARCHING ON.
auto now() -> std::size_t {
  return std::chrono::steady_clock::now().time_since_epoch().count();
}


// Self-explanatory, but checks if the entire string is a valid positive whole number.
auto is_number(const std::string& value) -> bool {
  for (const auto& i : value)
    if (!std::isdigit(i))
      return false;
  return (value.size() > 0);
}


// A less-bad-but-still-not-good implementation of Python's repr.
auto repr(const std::string& value) -> std::string {
  std::ostringstream oss;
  oss << std::quoted(value);
  return oss.str();
}

// A less-bad-but-still-not-good implementation of Python's repr.
auto repr(std::size_t value) -> std::string {
  std::ostringstream oss;
  oss << value;
  return oss.str();
}

// Join strings with separator
auto join(const std::vector<std::string>& vec, const std::string& sep) -> std::string {
  std::ostringstream oss;
  if (vec.size() > 1) {
    for (std::size_t ix = 0; ix < (vec.size() - 1); ++ix) {
      oss << vec.at(ix) << sep;
    }
  }
  oss << vec.back();
  return oss.str();
}

// Join repr'ed strings with separator
auto repr_join(const std::vector<std::string>& vec, const std::string& sep) -> std::string {
  std::vector<std::string> cev;
  cev.reserve(vec.size());
  for (const auto& item : vec) {
    cev.emplace_back(repr(item));
  }
  return join(cev, sep);
}


// Returns true if we lack read access.
auto file_is_unreadable(const std::filesystem::path& path) -> bool {
  std::filesystem::perms p = std::filesystem::status(path).permissions();
  std::filesystem::perms none = std::filesystem::perms::none;
  return (none == ((std::filesystem::perms::owner_read | std::filesystem::perms::group_read | std::filesystem::perms::others_read) & p));
}


/* Formatting helpers */

auto ftime_ns(std::size_t nanoseconds) -> std::string {
  long double ns = nanoseconds;
  std::vector<std::string> units = {"ns", "\u00b5s", "ms", "s"};
  std::size_t uix = 0;
  while (uix < (units.size() - 1) and ns >= 1000) {
    ns /= 1000;
    uix++;
  }
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(std::min<int>(uix, 2)) << ns << " " << units.at(uix);
  return oss.str();
}

// Convert nanoseconds into a more human-friendly format.
auto ftime(std::size_t seconds) -> std::string {
  return ftime_ns(seconds * 1'000'000'000);
}


// Convert bytes into a more human-friendly format.
auto fsize(std::size_t bytes, bool use_binary_prefix) -> std::string {
  std::string unit = "B";
  std::vector<std::string> units{"kB", "MB", "GB", "TB", "PB", "ZB", "YB"};
  double nbytes = bytes;
  std::size_t base = 1000;
  std::size_t prec = 0;

  if (use_binary_prefix) {
    units = {"KiB", "MiB", "GiB", "TiB", "PiB", "ZiB", "YiB"};
    base = 1024;
  }

  while (nbytes > base and not units.empty()) {
    unit = units.front();
    units.erase(units.begin());
    nbytes /= base;
    if (prec < 2)
      prec++;
  }

  std::ostringstream oss;
  oss << std::fixed << std::setprecision(prec) << nbytes << " " << unit;
  return oss.str();
}

auto hexdigest(std::uint64_t hash) -> std::string {
  std::ostringstream oss;
  oss << std::setbase(16) << std::setfill('0') << std::setw(16) << hash;
  return oss.str();
}
