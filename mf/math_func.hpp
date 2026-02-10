#ifndef MF_MATH_FUNC_HPP
#define MF_MATH_FUNC_HPP

#include <memory>

namespace mf {

typedef enum { CONSTANT, OPERATION, VARIABLE } NodeType;

typedef enum {
  UNKNOWN,
  ADD,
  NEG,
  MUL,
  INV,
  POW,
  EXP,
  LOG,
  SIN,
  COS,
  TAN
} OperationType;

class Node {
public:
  struct Impl;

  Node(NodeType node_t, OperationType op_t, float val);

  float value() const;
  float grad() const;

  float evaluate() const;
  void numeric_diff() const; // return evaluate(); get grad by x.grad()
  Node derivative() const;

  Node &operator=(float val);
  Node operator-() const;
  Node operator+(const Node &rhs) const;
  Node operator-(const Node &rhs) const;
  Node operator*(const Node &rhs) const;
  Node operator/(const Node &rhs) const;

  friend Node pow(const Node &x, const Node &y);
  friend Node exp(const Node &x);
  friend Node log(const Node &x);
  friend Node sin(const Node &x);
  friend Node cos(const Node &x);
  friend Node tan(const Node &x);

  static Node constant(const float val) { return {CONSTANT, UNKNOWN, val}; }
  static Node variable() { return {VARIABLE, UNKNOWN, 0}; }

private:
  explicit Node(const std::shared_ptr<Impl> &impl) : impl(impl) {}
  std::shared_ptr<Impl> impl;
};

Node pow(const Node &x, const Node &y);
Node exp(const Node &x);
Node log(const Node &x);
Node sin(const Node &x);
Node cos(const Node &x);
Node tan(const Node &x);

} // namespace mf

#endif // MF_MATH_FUNC_HPP
