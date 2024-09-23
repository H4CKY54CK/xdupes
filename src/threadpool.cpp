#include "threadpool.hpp"


// Manager for threads. Loops N threads. Each thread pulls tasks from the queue.
ThreadPool::ThreadPool(std::size_t threads) : max_workers(threads) {}

auto ThreadPool::start() -> void {
  std::size_t upper = std::thread::hardware_concurrency();
  max_workers = std::min(std::max<std::size_t>(max_workers, 0), upper);
  if (max_workers == 0) {
    max_workers = upper / 2;
  }

  if (!threads.empty()) {
    throw std::logic_error("ThreadPool::start() on an active ThreadLoop instance");
  }
  for (std::size_t ix = 0; ix < max_workers; ix++) {
    threads.emplace_back(&ThreadPool::loop, this);
  }
}

auto ThreadPool::loop() -> void {
  XXH3_state_t* const state = XXH3_createState();
  if (state == nullptr) {
    abort();
  }
  std::ifstream ifs;
  std::array<char, 1048576> buffer;

  while (true) {
    std::string task;
    if (XXH3_128bits_reset(state) == XXH_ERROR) {
      abort();
    }

    {
      std::unique_lock<std::mutex> lock(tasks_mutex);
      condition.wait(lock, [this] { return !tasks.empty() || should_terminate; });
      if (should_terminate) {
        // Still need to free the hash's state
        break;
      }
      task = tasks.front();
      tasks.pop();
    }

    ifs = std::ifstream(task, std::ifstream::binary);
    if (ifs.is_open()) {
      while (ifs.good()) {
        ifs.read(buffer.data(), 1048576);
        if (XXH3_128bits_update(state, buffer.data(), ifs.gcount()) == XXH_ERROR) {
          abort();
        }
      }
      ifs.close();
    }

    XXH128_hash_t hash = XXH3_128bits_digest(state);
    {
      std::unique_lock<std::mutex> lock(results_mutex);
      results[hash.low64][hash.high64].emplace_back(task);
    }
    {
      std::unique_lock<std::mutex> lock(total_mutex);
      total_done++;
    }
  }

  XXH3_freeState(state);
}

auto ThreadPool::enqueue(const std::string& path) -> void {
  {
    std::unique_lock<std::mutex> lock(tasks_mutex);
    tasks.push(path);
  }
  condition.notify_one();
}

auto ThreadPool::busy() -> bool {
  bool poolbusy;
  {
    std::unique_lock<std::mutex> lock(tasks_mutex);
    poolbusy = !tasks.empty();
  }
  return poolbusy;
}

auto ThreadPool::stop() -> void {
  {
    std::unique_lock<std::mutex> lock(tasks_mutex);
    should_terminate = true;
  }
  condition.notify_all();
  for (std::thread& active_thread : threads) {
    active_thread.join();
  }
  threads.clear();
}

auto ThreadPool::join() -> void {
  while (busy()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}
