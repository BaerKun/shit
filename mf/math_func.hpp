#ifndef MF_MATH_FUNC_HPP
#define MF_MATH_FUNC_HPP

#include <memory>
#include <vector>

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
  friend class Function;

  struct Impl;

  float value() const;
  float grad() const;

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
  Node(NodeType node_t, OperationType op_t, float val);
  explicit Node(std::shared_ptr<Impl> &&impl) : impl(std::move(impl)) {}

  std::shared_ptr<Impl> impl;
};

Node pow(const Node &x, const Node &y);
Node exp(const Node &x);
Node log(const Node &x);
Node sin(const Node &x);
Node cos(const Node &x);
Node tan(const Node &x);

class Function {
public:
  Function(Node expr) // NOLINT(*-explicit-constructor)
      : expr(std::move(expr)) {}

  Node get_expr() const { return expr; }

  float forward();
  void backward() const;
  Function derivative();

private:
  Node expr;
  std::vector<Node::Impl *> reverse_topo;
};

} // namespace mf

#endif // MF_MATH_FUNC_HPP
