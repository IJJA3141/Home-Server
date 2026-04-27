#include "../../src/ipc/buffer/buffer.hpp"
#include "../test.hpp"
#include <cstddef>

using namespace test;

constexpr void assert_equal(const int& l, const int& r)
{
  if (l != r) throw ComparisonException(l, r);
};

constexpr void assert_equal(const std::byte& l, const std::byte& r)
{
  if (l != r) throw ComparisonException("", "");
};

int ipc_ring_buffer(int _argc, char* _argv[])
{
  int _flag = 0, _i = 0, _j = 0;
  bool _exception_caught = false;

  SECTION("Correct initialization")
  {
    ipc::RingBuffer<std::byte, 10> buffer;

    REQUIRE("buffer should be empty upon creation", { assert_equal(buffer.size(), 0); })
    REQUIRE("buffer should have max cap upon creation", { assert_equal(buffer.capacity(), 10); })
    REQUIRE("read span should be empty upon creation", { assert_equal(buffer.read().size(), 0); })
  }

  SECTION("Write and acknowledge increases size")
  {
    ipc::RingBuffer<std::byte, 10> buffer;
    auto* write_ptr = buffer.write();

    write_ptr[0] = std::byte{1};
    write_ptr[1] = std::byte{2};
    write_ptr[2] = std::byte{3};

    buffer.acknowledge(3);

    REQUIRE("size should reflect acknowledged bytes", { assert_equal(buffer.size(), 3); })
    REQUIRE("capacity should shrink accordingly", { assert_equal(buffer.capacity(), 7); })
  }

  SECTION("Read returns written data")
  {
    ipc::RingBuffer<std::byte, 10> buffer;
    auto* write_ptr = buffer.write();

    write_ptr[0] = std::byte{10};
    write_ptr[1] = std::byte{20};

    buffer.acknowledge(2);

    auto data = buffer.read();

    REQUIRE("read span should contain written bytes", {
      assert_equal(data.size(), 2);
      assert_equal(data[0], std::byte{10});
      assert_equal(data[1], std::byte{20});
    })
  }

  SECTION("Discard reduces readable size")
  {
    ipc::RingBuffer<std::byte, 10> buffer;
    auto* write_ptr = buffer.write();

    write_ptr[0] = std::byte{1};
    write_ptr[1] = std::byte{2};
    write_ptr[2] = std::byte{3};

    buffer.acknowledge(3);

    buffer.discard(2);

    REQUIRE("size should shrink after discard", { assert_equal(buffer.size(), 1); })
    REQUIRE("remaining data should be correct", {
      auto data = buffer.read();
      assert_equal(data[0], std::byte{3});
    })
  }

  SECTION("Clear resets buffer state")
  {
    ipc::RingBuffer<std::byte, 10> buffer;
    auto* write_ptr = buffer.write();

    write_ptr[0] = std::byte{42};
    buffer.acknowledge(1);

    buffer.clear();

    REQUIRE("size should be zero after clear", { assert_equal(buffer.size(), 0); })
    REQUIRE("capacity should reset after clear", { assert_equal(buffer.capacity(), 10); })
  }

  SECTION("Discarding everything empties buffer")
  {
    ipc::RingBuffer<std::byte, 10> buffer;
    auto* write_ptr = buffer.write();

    write_ptr[0] = std::byte{1};
    write_ptr[1] = std::byte{2};

    buffer.acknowledge(2);
    buffer.discard(2);

    REQUIRE("size should return to zero", { assert_equal(buffer.size(), 0); })
    REQUIRE("capacity should return to full", { assert_equal(buffer.capacity(), 10); })
  }

  SECTION("Repeated write discard cycles")
  {
    ipc::RingBuffer<std::byte, 10> buffer;
    for (int i = 0; i < 1000; ++i)
    {
      auto* w = buffer.write();
      w[0] = std::byte{1};
      buffer.acknowledge(1);

      buffer.discard(1);
    }

    REQUIRE("buffer should still be valid", {
      assert_equal(buffer.size(), 0);
      assert_equal(buffer.capacity(), 10);
    })
  }

  SECTION("Write wraps correctly when reaching end")
  {
    ipc::RingBuffer<std::byte, 10> buffer;

    auto* w = buffer.write();

    for (size_t i = 0; i < 8; ++i)
      w[i] = static_cast<std::byte>(i);

    buffer.acknowledge(8);
    buffer.discard(8);

    // head now near boundary, next write should wrap
    auto* w2 = buffer.write();

    for (size_t i = 0; i < 5; ++i)
      w2[i] = static_cast<std::byte>(10 + i);

    buffer.acknowledge(5);

    auto data = buffer.read();

    REQUIRE("wrapped write should preserve order", {
      assert_equal(data.size(), 5);
      for (size_t i = 0; i < 5; ++i)
        assert_equal(data[i], static_cast<std::byte>(10 + i));
    })
  }

  SECTION("Discard wraps correctly at boundary")
  {
    ipc::RingBuffer<std::byte, 10> buffer;

    auto* w = buffer.write();

    for (size_t i = 0; i < 9; ++i)
      w[i] = static_cast<std::byte>(i);

    buffer.acknowledge(9);

    buffer.discard(8); // tail near end

    auto data = buffer.read();

    REQUIRE("remaining element should be correct", {
      assert_equal(data.size(), 1);
      assert_equal(data[0], std::byte{8});
    })

    buffer.discard(1);

    REQUIRE("buffer should be empty after wrap discard", { assert_equal(buffer.size(), 0); })
  }

  SECTION("Read across wrap boundary remains contiguous")
  {
    ipc::RingBuffer<std::byte, 10> buffer;

    auto* w = buffer.write();

    for (size_t i = 0; i < 8; ++i)
      w[i] = static_cast<std::byte>(i);

    buffer.acknowledge(8);
    buffer.discard(5);

    auto* w2 = buffer.write();

    w2[0] = std::byte{100};
    w2[1] = std::byte{101};
    w2[2] = std::byte{102};

    buffer.acknowledge(3);

    auto data = buffer.read();

    REQUIRE("read order should remain correct", {
      assert_equal(data.size(), 6);

      assert_equal(data[0], std::byte{5});
      assert_equal(data[1], std::byte{6});
      assert_equal(data[2], std::byte{7});

      assert_equal(data[3], std::byte{100});
      assert_equal(data[4], std::byte{101});
      assert_equal(data[5], std::byte{102});
    })
  }

  SECTION("Buffer correctly distinguishes full vs empty")
  {
    ipc::RingBuffer<std::byte, 10> buffer;

    auto* w = buffer.write();

    for (size_t i = 0; i < 10; ++i)
      w[i] = static_cast<std::byte>(i);

    buffer.acknowledge(10);

    REQUIRE("buffer should report full", {
      assert_equal(buffer.size(), 10);
      assert_equal(buffer.capacity(), 0);
    })

    buffer.discard(10);

    REQUIRE("buffer should report empty after full discard", {
      assert_equal(buffer.size(), 0);
      assert_equal(buffer.capacity(), 10);
    })
  }

  SECTION("Multiple wrap cycles maintain correctness")
  {
    ipc::RingBuffer<std::byte, 10> buffer;

    for (int cycle = 0; cycle < 200; ++cycle)
    {
      auto* w = buffer.write();

      for (size_t i = 0; i < 6; ++i)
        w[i] = static_cast<std::byte>(i);

      buffer.acknowledge(6);

      auto data = buffer.read();

      for (size_t i = 0; i < 6; ++i)
        assert_equal(data[i], static_cast<std::byte>(i));

      buffer.discard(6);

      assert_equal(buffer.size(), 0);
    }
  }

  SECTION("Partial wrap with remaining unread data")
  {
    ipc::RingBuffer<std::byte, 10> buffer;

    auto* w = buffer.write();

    for (size_t i = 0; i < 7; ++i)
      w[i] = static_cast<std::byte>(i);

    buffer.acknowledge(7);

    buffer.discard(4);

    auto* w2 = buffer.write();

    w2[0] = std::byte{50};
    w2[1] = std::byte{51};
    w2[2] = std::byte{52};

    buffer.acknowledge(3);

    auto data = buffer.read();

    REQUIRE("data should remain ordered after wrap", {
      assert_equal(data.size(), 6);

      assert_equal(data[0], std::byte{4});
      assert_equal(data[1], std::byte{5});
      assert_equal(data[2], std::byte{6});

      assert_equal(data[3], std::byte{50});
      assert_equal(data[4], std::byte{51});
      assert_equal(data[5], std::byte{52});
    })
  }

  SECTION("Capacity behaves correctly near wrap boundary")
  {
    ipc::RingBuffer<std::byte, 10> buffer;

    auto* w = buffer.write();

    for (size_t i = 0; i < 9; ++i)
      w[i] = static_cast<std::byte>(i);

    buffer.acknowledge(9);

    REQUIRE("capacity should reflect single remaining slot", { assert_equal(buffer.capacity(), 1); })

    buffer.discard(5);

    REQUIRE("capacity should increase after discard", { assert_equal(buffer.capacity(), 6); })
  }
  ENDSECTION;

  return _flag;
}
