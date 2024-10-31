#include "lib_xcore"
#include <cassert>
#include <iostream>

// todo: test is incorrect.

void test_deque() {
  xcore::container::deque_t<int, 5> d;

  // Test push_back and push_front
  d.push_back(1);
  d.push_back(2);
  d.push_front(0);

  // Test front and back access
  assert(d.front() && *d.front() == 0);
  assert(d.back() && *d.back() == 2);

  // Test pop operations
  {
    auto x = d.pop_front();
    assert(x && *x == 0);
    x = d.pop_back();
    assert(x && *x == 2);
  }

  // Test push_back_force when full
  d.push_back(3);
  d.push_back(4);
  d.push_back(5);
  d.push_back(6);
  d.push_back(7);        // Deque is now full
  d.push_back_force(8);  // This should overwrite the oldest element
  assert(d.front() && *d.front() == 4);

  std::cout << "Deque test passed." << std::endl;
}

void test_queue() {
  xcore::container::queue_t<int, 5> q;

  // Test push and pop
  q.push(1);
  q.push(2);
  q.push(3);
  assert(q.peek() && *q.peek() == 1);  // Check front element

  // Test pop and peek
  assert(q.pop().has_value() && q.pop().value() == 1);
  assert(q.peek().has_value() && q.peek().value() == 2);

  // Test push_force when full
  q.push(4);
  q.push(5);
  q.push(6);
  q.push_force(7);  // Overwrite front when full
  assert(q.peek().has_value() && q.peek().value() == 3);

  std::cout << "Queue test passed." << std::endl;
}

void test_stack() {
  xcore::container::stack_t<int, 5> s;

  // Test push and pop
  s.push(1);
  s.push(2);
  s.push(3);
  assert(s.peek().has_value() && s.peek().value() == 3);  // Check top element

  // Test pop and peek
  assert(s.pop().has_value() && s.pop().value() == 3);
  assert(s.peek().has_value() && s.peek().value() == 2);

  // Test push_force when full
  s.push(4);
  s.push(5);
  s.push(6);
  s.push_force(7);  // Overwrite bottom when full
  assert(s.peek().has_value() && s.peek().value() == 7);

  std::cout << "Stack test passed." << std::endl;
}

int main() {
  test_deque();
  test_queue();
  test_stack();
  return 0;
}
