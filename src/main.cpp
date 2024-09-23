#include <csignal>

#include "logging.hpp"
#include "parser.hpp"
#include "progressbar.hpp"
#include "threadpool.hpp"
#include "utils.hpp"



void restore_terminal(int s);

auto create_parser() -> parsing::ArgumentParser;

auto main(int argc, char** argv) -> int {

  // Set handler for signal as early as possible.
  std::signal(SIGTERM, restore_terminal);
  std::signal(SIGINT, restore_terminal);

  namespace fs = std::filesystem;

  // It was getting hard to read with this monstrosity in the way.
  auto parser = create_parser();
  logging::Logger& logger = logging::get_logger("xdupes");

  auto options = parser.parse_args(argc - 1, argv + 1);

  auto quiet = options["quiet"].as_bool();
  auto silent = options["silent"].as_bool();
  auto progress = options["progress"].as_bool();

  // Get a better argument parser.
  if (!is_number(options["threads"].as_string())) {
    logger.error("threads must be a positive integer");
    std::quick_exit(1);
  }

  TimeStats stats;

  // Start timer
  std::size_t t0 = now();
  stats.proc_start = t0;

  // Log time for parsing
  stats.parsing = now() - t0;

  logger.set_level(options["loglevel"].as_size_t());

  if (options["sources"].as_strings().size() == 0) {
    logger.error("missing SOURCE(s) arguments");
    return 1;
  }

  // Reset timer
  t0 = now();

  std::deque<fs::path> stack;
  std::vector<std::string> sources = options["sources"].as_strings();
  stack.insert(stack.end(), std::make_move_iterator(sources.begin()), std::make_move_iterator(sources.end()));

  fs::path source;
  std::map<std::size_t,std::vector<std::string>> sizes;
  std::set<fs::path> seen;

  std::size_t total_walked = 0;
  std::size_t total_hashed = 0;

  // Hide cursor, save cursor position
  if (progress) {
    std::cout << "\x1b[?25l\x1b[s";
  }

  while (!stack.empty()) {
    source = stack.front();
    stack.pop_front();

    if (progress) {
      std::cout << "Files Walked: " << total_walked << "\x1b[u";
    }

    if (file_is_unreadable(source)) {
      logger.warn("permission denied: " + repr(source.native()));
      continue;
    }

    if (seen.count(source)) {
      continue;
    }

    seen.insert(source);


    if (!fs::exists(source)) {
      logger.warn("invalid directory: " + repr(source));
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

  // if (progress) {
  //   // We double clear in case we happened to spill over onto the next line. We clear that line, return to ours, and
  //   // then clear our line. If we didn't spill over, then this just clears twice and that's harmless.
  //   std::cout << "\x1b[2K\x1b[u\x1b[2K";
  // }

  if (options["skip-empty"].as_bool()) {
    sizes[0].clear();
  }


  // Log time for filesystem traversal
  stats.filesystem = now() - t0;

  for (const auto& [size, files] : sizes) {
    if (files.size() == 1) {
      continue;
    }
    total_hashed += files.size();
  }

  // Reset timer
  t0 = now();

  ThreadPool tp(options["threads"].as_size_t());

  ProgressBar pbar(total_hashed);
  pbar.set_prefix("Queueing tasks: ");

  tp.start();

  if (progress) {
    pbar.update(0);
    std::cout << pbar.bar << '\r';
  }

  for (const auto& [size, files] : sizes) {
    if (files.size() == 1) {
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
  // if (progress) {
  //   // We double clear in case we happened to spill over onto the next line. We clear that line, return to ours, and
  //   // then clear our line. If we didn't spill over, then this just clears twice and that's harmless.
  //   std::cout << "\x1b[2K\x1b[u\x1b[2K";
  // }


  pbar.reset();
  pbar.set_prefix("Hashing files:  ");


  if (progress) {
    while (tp.busy()) {
      {
        std::unique_lock<std::mutex> lock(tp.total_mutex);
        pbar.set_progress(tp.total_done);
      }
      std::cout << pbar.bar << "\x1b[u";
      std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    if (pbar.subtotal < tp.total_done) {
      pbar.set_progress(total_hashed);
      std::cout << pbar.bar << "\x1b[u";
    }
    std::cout << "\x1b[2K\x1b[u\x1b[2K\x1b[?25h";
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


// Signal handler for when progress bar gets cancelled
void restore_terminal(int s) {
  // Clear line, restore cursor position, unhide cursor.
  std::cout << "\x1b[2K\x1b[u\x1b[?25h\x1b[2K" << std::endl;
  std::quick_exit(1);
}


auto create_parser() -> parsing::ArgumentParser {
  parsing::ArgumentParser parser("xdupes");

  parser.add_argument(
    parsing::Argument({"sources"})
      .nargs("+")
      .help("Path(s) to SOURCE directories to check for file duplicates in.")
  );

  parser.add_argument(
    parsing::Argument({"--threads", "-t"})
      .default_value("1")
      .help("How many threads to use.")
  );
  parser.add_argument(
    parsing::Argument({"--recursive", "-r"})
      .action("store_true")
      .help("Walk all subdirectories of SOURCES.")
  );
  parser.add_argument(
    parsing::Argument({"--noempty", "--skip-empty"})
      .action("store_true")
      .help("Skip empty files (all empty files hash to the same value, so it's worth skipping them. This may default to true in the future.).")
  );

  parser.add_argument(
    parsing::Argument({"--quiet", "-q"})
      .action("store_true")
      .help("Don't display the files (still displays anything else that is normally displayed).")
  );
  parser.add_argument(
    parsing::Argument({"--silent", "-s"})
      .action("store_true")
      .help("Show no output.")
  );

  parser.add_argument(
    parsing::Argument({"--loglevel"})
      .dest("loglevel")
      .default_value("30")
      .help("Adjust the logging level manually.")
  );
  parser.add_argument(
    parsing::Argument({"--debug"})
      .dest("loglevel")
      .action("store_const")
      .const_value("10")
      .help("Show all output.")
  );

  parser.add_argument(
    parsing::Argument({"--timed"})
      .action("store_true")
      .help("Show elapsed time from the moment parsing has finished to the moment the program is done doing its work.")
  );
  parser.add_argument(
    parsing::Argument({"--wasted", "--wasted-space"})
      .action("store_true")
      .help("Show total_hashed space taken up by duplicate files (not including the unique one).")
  );
  parser.add_argument(
    parsing::Argument({"--si", "--binary"})
      .action("store_true")
      .help("Use binary prefixes (KiB, MiB, etc.) instead of the default (KB, MB, etc.).")
  );
  parser.add_argument(
    parsing::Argument({"--progress"})
      .action("store_true")
      .help("Show a helpful progress bar instead of the nothing that currently gets shown.")
  );
  parser.add_argument(
    parsing::Argument({"--zero"})
      .dest("separator")
      .default_value("\n")
      .const_value("\0")
      .action("store_const")
      .help("Use a null-terminator instead of newline to separate files and groups in the output.")
  );
  return parser;
}
