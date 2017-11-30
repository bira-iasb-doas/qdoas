#include <iterator>
#include <string>
#include <stdexcept>
#include <memory>
#include <iostream>

#include <dirent.h>

class dir_iter : public std::iterator<std::input_iterator_tag, std::string> {
private:
  std::shared_ptr<DIR> stream;
  std::string current;
  void advance() {
    auto entry = readdir(stream.get());
    if (entry) {
      current = entry->d_name;
    } else {
      // end of dir
      this->stream.reset();
      current = "";
    }
  }
public:
  dir_iter(std::string path) : stream(opendir(path.c_str()), [](DIR *dir) {closedir(dir); }) {
    if (!stream)
      throw std::runtime_error("failed to open directory '" + path + "'");
    this->advance();
  };
  dir_iter() : stream(), current("") {
  };
  std::string& operator*() { return this->current; }
  std::string * operator->() { return &this->current; }
  dir_iter& operator++() { this->advance(); return *this; }
  dir_iter operator++(int) { //
    dir_iter rc(*this);
    this->operator++();
    return rc;
  }
  bool operator==(dir_iter const& other) const {
    return (this->stream == other.stream && this->current == other.current);
  }
  bool operator!=(dir_iter const& other) const {
    return !(*this == other);
  }

  // begin() & end() functions for range-based for loops:
  dir_iter begin() {
    return *this;
  }
  static dir_iter end() {
    return dir_iter();
  }

};
