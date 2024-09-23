#pragma once

#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <map>
#include <queue>
#include <thread>

#include "xxh3.h"


// Thread pool manager
class ThreadPool {
public:
  ThreadPool(std::size_t);
  void start();
  void enqueue(const std::string&);
  void stop();
  bool busy();
  void join();
  std::map<XXH64_hash_t, std::map<XXH64_hash_t, std::vector<std::string>>> results;
  std::size_t total_done = 0;
  std::mutex total_mutex;
private:
  void loop();
  std::size_t max_workers = 1;
  std::vector<std::thread> threads;
  std::queue<std::string> tasks;

  bool should_terminate = false;

  std::mutex tasks_mutex;
  std::mutex results_mutex;

  std::condition_variable condition;
};
