#pragma once

#include <cstddef>
#include <cstring>
#include <format>
#include <span>
#include <stdexcept>
#include <sys/mman.h>

namespace ipc
{

template <typename T, size_t S> class RingBuffer
{
public:
  RingBuffer();

  T* write();
  size_t capacity() const;
  void acknowledge(size_t written);

  std::span<const T> read() const;
  size_t size() const;
  void discard(size_t read);

  void clear();

private:
  size_t head_;   // offset to written end
  size_t tail_;   // offset to read end
  bool exhausted; // true if we read all T available
  T memory_[2 * S];
};

template <typename _T, size_t _S> RingBuffer<_T, _S>::RingBuffer() : head_(0), tail_(0), exhausted(true) {}

template <typename _T, size_t _S> _T* RingBuffer<_T, _S>::write() { return this->memory_ + head_; }

template <typename _T, size_t _S> size_t RingBuffer<_T, _S>::capacity() const
{
  if (this->head_ > this->tail_) return _S - this->head_ + this->tail_;
  if (this->head_ < this->tail_) return this->tail_ - this->head_;
  return this->exhausted ? _S : 0;
}

template <typename _T, size_t _S> void RingBuffer<_T, _S>::acknowledge(size_t _w)
{
  if (_w == 0) return;
  if (_w > capacity()) throw std::out_of_range(std::format("{} is out of range for capacity {}", _w, capacity()));
  if ((this->head_ += _w) >= _S) std::memcpy(this->memory_, this->memory_ + _S, (this->head_ -= _S));
  this->exhausted = false;
}

template <typename _T, size_t _S> std::span<const _T> RingBuffer<_T, _S>::read() const
{
  return std::span(this->memory_ + this->tail_, this->memory_ + this->tail_ + this->size());
}

template <typename _T, size_t _S> size_t RingBuffer<_T, _S>::size() const
{
  if (this->tail_ > this->head_) return _S - this->tail_ + this->head_;
  if (this->tail_ < this->head_) return this->head_ - this->tail_;
  return this->exhausted ? 0 : _S;
}

template <typename _T, size_t _S> void RingBuffer<_T, _S>::discard(size_t _r)
{
  if (_r == 0) return;
  if (_r > size()) throw std::out_of_range(std::format("{} is out of range for size {}", _r, capacity()));
  if ((this->tail_ += _r) >= _S) this->tail_ -= _S;
  this->exhausted = this->tail_ == this->head_;
}

template <typename _T, size_t _S> void RingBuffer<_T, _S>::clear()
{
  this->head_ = this->tail_ = 0;
  this->exhausted = true;
}

} // namespace ipc
