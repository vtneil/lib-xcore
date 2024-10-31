#include <iostream>
#include <cassert>
#include "lib_xcore"

void test_push_and_peek() {
  xcore::container::byte_buffer_t<8> buffer;

  //

  const unsigned char data1[]     = "abcd";
  const unsigned char data2[]     = "efgh";
  unsigned char       peek_buf[4] = {};

  // Test normal push and peek without wrap
  assert(buffer.push(data1, 4));
  auto result = buffer.peek(peek_buf, 4);
  assert(result.has_value() && std::equal(peek_buf, peek_buf + 4, data1));
  std::cout << "test_push_and_peek passed without wrap" << std::endl;

  // Push more data and check wrap-around
  assert(buffer.push(data2, 4));
  assert(buffer.size() == 8);  // Buffer is full now

  // Peek entire buffer content (with wrap-around)
  unsigned char full_buf[8] = {};
  result                    = buffer.peek(full_buf, 8);
  assert(result.has_value());
  assert(std::equal(full_buf, full_buf + 4, data1));
  assert(std::equal(full_buf + 4, full_buf + 8, data2));
  std::cout << "test_push_and_peek passed with wrap-around" << std::endl;
}

void test_push_force() {
  xcore::container::byte_buffer_t<8> buffer;

  const unsigned char         data1[]     = "abcdef";
  const unsigned char         data2[]     = "gh";
  unsigned char               peek_buf[8] = {};

  // Fill the buffer
  assert(buffer.push(data1, 6));

  // Force push additional data causing wrap-around overwrite
  assert(buffer.push_force(data2, 4));  // Should overwrite the front "ab"

  auto result = buffer.peek(peek_buf, 8);
  assert(result.has_value());

  // Expected buffer state: "cdefgh" (last 4 bytes are new)
  const unsigned char expected[] = "cdefgh";
  assert(std::equal(peek_buf, peek_buf + 6, expected));
  std::cout << "test_push_force passed with forced overwrite" << std::endl;
}

void test_single_byte_operations() {
  xcore::container::byte_buffer_t<8> buffer;
  unsigned char               peek_buf[8] = {};

  // Test single-byte push and force
  assert(buffer.push('x'));
  assert(buffer.push('y'));
  assert(buffer.size() == 2);

  auto result = buffer.peek(peek_buf, 2);
  assert(result && peek_buf[0] == 'x' && peek_buf[1] == 'y');

  // Force a byte push to fill
  for (int i = 0; i < 7; ++i) {
    buffer.push('z' + i);
  }

  // Expected last state in buffer after pushes
  const unsigned char expected[] = {'x', 'y', 'z', '{', '|', '}', '~'};
  result                         = buffer.peek(peek_buf, 7);
  assert(result.has_value());
  assert(std::equal(peek_buf, peek_buf + 7, expected));

  std::cout << "test_single_byte_operations passed" << std::endl;
}

void test_pop() {
  xcore::container::byte_buffer_t<8> buffer;

  const unsigned char         data1[]    = "abcdef";
  const unsigned char         data2[]    = "gh";
  unsigned char               pop_buf[8] = {};

  // Fill the buffer partially
  assert(buffer.push(data1, 6));
  assert(buffer.size() == 6);

  // Pop a portion of the buffer
  auto result = buffer.pop(pop_buf, 3);
  assert(result.has_value());                       // Should succeed
  assert(std::equal(pop_buf, pop_buf + 3, "abc"));  // Check that "abc" was popped
  assert(buffer.size() == 3);                       // Size should be reduced

  // Add more data to wrap around
  assert(buffer.push(data2, 2));
  assert(buffer.size() == 5);

  // Pop across the wrap-around boundary
  result = buffer.pop(pop_buf, 5);
  assert(result.has_value());                         // Should succeed
  assert(std::equal(pop_buf, pop_buf + 5, "defgh"));  // Check that "defgh" was popped
  assert(buffer.size() == 0);                         // Buffer should be empty now

  std::cout << "test_pop passed" << std::endl;
}

int main() {
  test_push_and_peek();
  test_push_force();
  test_single_byte_operations();
  test_pop();
  std::cout << "All tests passed!" << std::endl;
  return 0;
}
