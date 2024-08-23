#include "parser.hpp"

bool Stream::operator>>(std::string &_string)
{
  if (this->end_ >= this->endOfSteam_) return false;

  for (; this->end_ != this->endOfSteam_; this->end_++) {
    switch (this->string_[this->end_]) {
    case ' ':
    case '\n':
      _string = this->string_.substr(this->begin_, this->end_ - this->begin_);
      this->begin_ = ++this->end_;
      return true;
    case '\r':
      if (++this->end_ >= this->endOfSteam_ || this->string_[this->end_] == '\n') {
        _string = this->string_.substr(this->begin_, --this->end_ - this->begin_);
        this->begin_ = this->end_ += 2;
        return true;
      } else {
        _string = this->string_.substr(this->begin_, --this->end_ - this->begin_);
        this->begin_ = ++this->end_;
        return true;
      }
    default:
      break;
    }
  }

  _string = this->string_.substr(this->begin_, ++this->end_ - this->begin_);
  return true;
};
