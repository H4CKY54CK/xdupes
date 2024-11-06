#include "parsing.hpp"
#include "logging.hpp"
#include "progressbar.hpp"
#include "threadpool.hpp"
#include "utils.hpp"


auto create_parser() -> parsing::ArgumentParser;

auto main(int argc, char** argv) -> int {

  // Save cursor position (allows cleanup function to be indiscriminate)
  std::cout << "\x1b[s";

  // Set handler for signal as early as possible.
  std::signal(SIGTERM, restore_terminal);
  std::signal(SIGINT, restore_terminal);

  namespace fs = std::filesystem;

  TimeStats stats;

  // Start timer
  std::size_t t0 = now();
  stats.proc_start = t0;

  // It was getting hard to read with this monstrosity in the way.
  auto parser = create_parser();
  logging::Logger& logger = logging::get_logger("xdupes");

  auto options = parser.parse_args(argc - 1, argv + 1);

  // Log time for parsing
  stats.parsing = now() - t0;

  // Convenience
  auto quiet = options["quiet"].as_bool();
  auto silent = options["silent"].as_bool();
  auto progress = options["progress"].as_bool();

  // Set logging for logger, prior to using the logger
  logger.set_level(options["loglevel"].as_size_t());

  // Get a better argument parser.
  // Remove this after typing is added to the Arguments
  if (!is_number(options["threads"].as_string())) {
    logger.error("threads must be a positive integer");
    return 1;
  }

  // Really, the ArgumentParser should be handling this
  if (options["sources"].as_strings().size() == 0) {
    logger.error("missing SOURCE(s) arguments");
    return 1;
  }

  // Reset timer
  t0 = now();

  std::vector<std::string> sources = options["sources"].as_strings();

  std::sort(sources.begin(), sources.end(), [](auto left, auto right) { return left < right; });

  std::deque<std::string> stack;
  std::string path;

  for (const auto& item : sources) {
    path = item.substr(0, item.find_last_not_of("/") + 1);
    for (const auto& prefix : stack) {
      if (path.substr(0, prefix.size()) == prefix) {
        goto next_item;
      }
    }
    stack.emplace_back(path);
    next_item:
    continue;
  }

  fs::path source;
  std::map<std::size_t,std::vector<std::string>> sizes;

  std::size_t total_walked = 0;
  std::size_t total_hashed = 0;

  // Hide cursor
  if (progress) {
    std::cout << "\x1b[?25l\x1b[";
  }

  while (!stack.empty()) {
    source = stack.front();
    stack.pop_front();

    if (progress) {
      std::cout << "Files Walked: " << total_walked << "\x1b[u";
    }

    if (!fs::exists(source)) {
      logger.warn("invalid directory: " + repr(source));
      continue;
    }

    if (file_is_unreadable(source)) {
      logger.warn("permission denied: " + repr(source.native()));
      continue;
    }

    for (const auto& dir : fs::directory_iterator(source)) {
      if (dir.is_symlink()) {
        continue;
      }
      if (dir.is_directory()) {
        if (options["recursive"].as_bool()) {
          stack.emplace_back(dir.path());
        }
        continue;
      }
      if (dir.is_regular_file()) {
        total_walked++;
        sizes[dir.file_size()].emplace_back(dir.path());
        continue;
      }
    }
  }

  // For some reason, if I do this inside the [size,files] loops, I get string error
  if (options["skip-empty"].as_bool()) {
    sizes[0].clear();
  }

  // Log time for filesystem traversal
  stats.filesystem = now() - t0;


  for (const auto& [size, files] : sizes) {
    if (files.size() < 2) {
      continue;
    }
    total_hashed += files.size();
  }

  // Reset timer
  t0 = now();

  ThreadPool tp(options["threads"].as_size_t());

  ProgressBar pbar(total_hashed);

  tp.start();

  if (progress) {
    pbar.set_prefix("Queueing tasks: ");
  }

  for (const auto& [size, files] : sizes) {
    if (files.size() < 2) {
      continue;
    }
    for (const auto& item : files) {
      tp.enqueue(item);
      if (progress) {
        pbar.update(1);
        std::cout << pbar.bar << "\x1b[u";
      }
    }
  }

  if (progress) {
    pbar.reset();
    pbar.set_prefix("Hashing files:  ");
  }

  if (progress) {
    std::size_t td;
    while (tp.busy()) {
      {
        std::unique_lock<std::mutex> lock(tp.total_mutex);
        td = tp.total_done;
      }
      pbar.set_progress(tp.total_done);
      std::cout << pbar.bar << "\x1b[u";
      if (td == total_hashed) {
        break;
      }
      std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    pbar.set_progress(total_hashed);
    std::cout << pbar.bar << "\x1b[2K\x1b[u\x1b[2K\x1b[?25h";
    std::cout.flush();
  }

  tp.join();
  tp.stop();

  // Log time for hashing
  stats.hashing = now() - t0;

  std::size_t total_wasted = 0;

  for (const auto& [high, inner] : tp.results) {
    for (const auto& [low, files] : inner) {
      if (files.size() < 2) {
        continue;
      }
      total_wasted += fs::file_size(files.at(0)) * (files.size() - 1);
      if (!quiet and !silent) {
        for (const auto& item : files) {
          std::cout << item << options["separator"].as_char();
        }
        std::cout << options["separator"].as_char();
      }
    }
  }

  if (options["wasted-space"].as_bool() and !silent) {
    std::cout << "Wasted space from duplicate files: " << fsize(total_wasted, options["binary"].as_bool()) << '\n';
  }

  stats.proc_stop = now();
  stats.overall = stats.proc_stop - stats.proc_start;

  if (options["timed"].as_bool() and !silent) {
    std::cout << "Elapsed time: " << ftime_ns(stats.overall) << "\n";
  }

  logger.debug("threads: " + options["threads"].as_string());
  logger.debug("total files found: " + std::to_string(total_walked));
  logger.debug("total files hashed: " + std::to_string(total_hashed));
  logger.debug("elapsed: parsing: " + ftime_ns(stats.parsing));
  logger.debug("elapsed: walking: " + ftime_ns(stats.filesystem));
  logger.debug("elapsed: hashing: " + ftime_ns(stats.hashing));
  logger.debug("elapsed: overall: " + ftime_ns(stats.overall));

  return 0;
}



auto create_parser() -> parsing::ArgumentParser {
  parsing::ArgumentParser parser = parsing::ArgumentParser::create_parser("hacky");
  parser.add_help(false);

  parsing::ActionGroup& positional_group = parser.add_argument_group("Positional");
  positional_group.add_argument("sources")
      .nargs("+")
      .help("Path(s) to SOURCE directories to check for file duplicates in.");

  parsing::ActionGroup& inner_group = parser.add_argument_group("General");
  inner_group.add_argument({"--threads", "-t"})
      .default_value("1")
      .help("How many threads to use.");
  inner_group.add_argument({"--recursive", "-r"})
      .action(parsing::actions::store_true)
      .help("Walk all subdirectories of SOURCES.");
  inner_group.add_argument({"--noempty", "--skip-empty"})
      .action(parsing::actions::store_true)
      .help("Skip empty files (all empty files hash to the same value, so it's worth skipping them. This may default to true in the future.).");

  inner_group.add_argument({"--si", "--binary"})
      .action(parsing::actions::store_true)
      .help("Use binary prefixes (KiB, MiB, etc.) instead of the default (KB, MB, etc.).");
  inner_group.add_argument({"--zero"})
      .dest("separator")
      .default_value("\n")
      .const_value("\0")
      .action(parsing::actions::store_const)
      .help("Use a null-terminator instead of newline to separate files and groups in the output.");

  parsing::ActionGroup& output_group = parser.add_argument_group("Output");
  output_group.add_argument({"--quiet", "-q"})
      .action(parsing::actions::store_true)
      .help("Don't display the files (still displays anything else that is normally displayed).");
  output_group.add_argument({"--silent", "-s"})
      .action(parsing::actions::store_true)
      .help("Show no output.");
  output_group.add_argument({"--loglevel"})
      .dest("loglevel")
      .default_value("30")
      .help("Adjust the logging level manually.");
  output_group.add_argument({"--debug"})
      .dest("loglevel")
      .action(parsing::actions::store_const)
      .const_value("10")
      .help("Show debug info.");

  parsing::ActionGroup& info_group = parser.add_argument_group("Informational");
  info_group.add_argument({"--progress"})
      .action(parsing::actions::store_true)
      .help("Show a helpful progress bar instead of the nothing that currently gets shown.");
  info_group.add_argument({"--timed"})
      .action(parsing::actions::store_true)
      .help("Show elapsed time from the moment parsing has finished to the moment the program is done doing its work.");
  info_group.add_argument({"--wasted", "--wasted-space"})
      .action(parsing::actions::store_true)
      .help("Show total space taken up by duplicate files (not including the unique one).");
  info_group.add_argument({"--help", "-h"})
      .action(parsing::actions::help)
      .help("Show this help menu and then exit.");

  return parser;
}
