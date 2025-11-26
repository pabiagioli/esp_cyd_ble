#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>

template<typename T, size_t Capacity>
class RingBuffer {
public:
  RingBuffer()
    : head_(0), tail_(0) {}

  // Push one element into the buffer.
  // Returns false if the buffer is full.
  inline bool push(const T& item) {
    size_t head = head_.load(std::memory_order_relaxed);
    size_t next = increment(head);

    if (next == tail_.load(std::memory_order_acquire)) {
      // buffer full
      return false;
    }

    buffer_[head] = item;
    head_.store(next, std::memory_order_release);
    return true;
  }

  // Pop one element from the buffer.
  // Returns false if buffer empty.
  inline bool pop(T& out) {
    size_t tail = tail_.load(std::memory_order_acquire);
    size_t head = head_.load(std::memory_order_acquire);

    if (tail == head) {
      // buffer empty
      return false;
    }

    out = buffer_[tail];
    tail_.store(increment(tail), std::memory_order_release);
    return true;
  }

  size_t pop(T* out, size_t count) {
    size_t t = tail_.load(std::memory_order_acquire);
    size_t h = head_.load(std::memory_order_acquire);

    size_t available = (h >= t) ? (h - t) : (Capacity - t + h);
    if (available == 0) return 0;

    size_t toRead = (count < available) ? count : available;

    size_t firstPart = std::min(toRead, Capacity - t);
    memcpy(out, buffer_ + t, firstPart * sizeof(T));

    if (toRead > firstPart) {
      memcpy(out + firstPart, buffer_, (toRead - firstPart) * sizeof(T));
    }

    tail_.store((t + toRead) % Capacity, std::memory_order_release);
    return toRead;
  }

  inline bool empty() const {
    return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
  }

  inline bool full() const {
    size_t next = increment(head_.load(std::memory_order_relaxed));
    return next == tail_.load(std::memory_order_acquire);
  }

  inline size_t size() const {
    size_t head = head_.load(std::memory_order_acquire);
    size_t tail = tail_.load(std::memory_order_acquire);
    return (head >= tail) ? (head - tail) : (Capacity - (tail - head));
  }

private:
  static constexpr size_t increment(size_t idx) {
    return (idx + 1) % Capacity;
  }

  T buffer_[Capacity];
  std::atomic<size_t> head_;  // producer writes here
  std::atomic<size_t> tail_;  // consumer reads here
};
