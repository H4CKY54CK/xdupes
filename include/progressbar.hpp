#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <vector>



class ProgressBar {
public:
  std::size_t total;
  std::size_t subtotal;
  std::string prefix = "Progress: ";
  std::string suffix = " ";
  std::string bar;
  winsize term_winsize_s;

  ProgressBar(std::size_t);

  void update_terminal_size();
  void set_progress(std::size_t amount);
  void reset();
  void update(int amount);
  void update();
  void display();
  void set_prefix(const std::string&);
  void set_suffix(const std::string&);
};
