#include "progressbar.hpp"



ProgressBar::ProgressBar(std::size_t total) : total(total), subtotal(0) {}

void ProgressBar::update_terminal_size() {
  ioctl(0, TIOCGWINSZ, &term_winsize_s);
}

// Set progress bar manually. Helpful for when you have to poll for updates and don't want to track the changes.
void ProgressBar::set_progress(std::size_t amount) {
  subtotal = amount;
  update(0);
}

// Convenience member function for easily resetting
void ProgressBar::reset() {
  set_progress(0);
}

// This only returns the bar text. It does not automatically return to the beginning of the line.
void ProgressBar::update(int amount) {
  subtotal += amount;
  update_terminal_size();

  // 8 = 2 for the surrounding "|", and 6 for the width of the percentage
  std::size_t columns = term_winsize_s.ws_col - (prefix.size() + suffix.size() + 8);

  // Progress in terms of percentage (convert both to float first)
  double l = subtotal;
  double r = total;
  double progress = 1;

  if (total > 0) {
    progress = l / r;
  }

  // How much to fill the bar
  std::size_t f = columns * progress;
  std::size_t uf = columns - f;
  std::string filled(f, ' ');
  std::string unfilled(uf, ' ');

  // Now make 'progress' a percentage
  progress *= 100.0;

  std::ostringstream oss;
  oss << prefix << "|" << "\x1b[42m" << filled << "\x1b[00m" << unfilled << "|" << suffix << std::fixed << std::setprecision(1) << std::setw(5) << progress << "%";
  bar = oss.str();
}

// Convenience member function for single updates.
void ProgressBar::update() {
  update(1);
}

// Convenience member function for automatically adding \r or \n and displaying the bar.
void ProgressBar::display() {
  char endchar = '\r';
  if (subtotal >= total) {
    endchar = '\n';
  }
  std::cout << bar << endchar;
}

// These are separate because I'd like to eventually implement a widget system for dynamic progress bar functionality.
void ProgressBar::set_prefix(const std::string& p) {
  prefix = p;
}

void ProgressBar::set_suffix(const std::string& p) {
  suffix = p;
}
