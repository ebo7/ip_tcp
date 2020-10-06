#include "wrapping_integers.hh"
#include <math.h>
// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
  uint64_t mod = (static_cast<uint64_t>(1)) << 32;
  return WrappingInt32{
    static_cast<uint32_t>((n + isn.raw_value()) % mod)
      };
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
  uint64_t e31 = static_cast<uint64_t>(1) << 31;
  uint64_t e32 = static_cast<uint64_t>(1) << 32;
  uint64_t mid = checkpoint + isn.raw_value();
  uint64_t num_cycle = mid / e32;
  if (n.raw_value() < e31){
    return n.raw_value() + num_cycle * e32 - isn.raw_value();
  }
  return n.raw_value() + (num_cycle - 1) * e32 - isn.raw_value();
  
}
