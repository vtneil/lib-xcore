#include "embedded_cpp"
#include <cassert>
#include <iostream>

void test_deque() {
  container::deque_t<int, 5> d;

  // Test push_back and push_front
  d.push_back(1);
  d.push_back(2);
  d.push_front(0);

  // Test front and back access
  assert(d.front().has_value() && d.front().value() == 0);
  assert(d.back().has_value() && d.back().value() == 2);

  // Test pop operations
  assert(d.pop_front().has_value() && d.pop_front().value() == 0);
  assert(d.pop_back().has_value() && d.pop_back().value() == 2);

  // Test push_back_force when full
  d.push_back(3);
  d.push_back(4);
  d.push_back(5);
  d.push_back(6);
  d.push_back(7);        // Deque is now full
  d.push_back_force(8);  // This should overwrite the oldest element
  assert(d.front().has_value() && d.front().value() == 4);

  std::cout << "Deque test passed." << std::endl;
}

void test_queue() {
  container::queue_t<int, 5> q;

  // Test push and pop
  q.push(1);
  q.push(2);
  q.push(3);
  assert(q.peek().has_value() && q.peek().value() == 1);  // Check front element

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
  container::stack_t<int, 5> s;

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
