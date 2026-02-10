#include "math_func.hpp"
#include <cmath>

namespace mf {
using NodeImpl = Node::Impl;
using NodeImplSharedPtr = std::shared_ptr<Node::Impl>;

struct Node::Impl {
  NodeType node_tp;
  OperationType op_tp;
  float value, grad;
  std::shared_ptr<Impl> operand1, operand2;

  Impl(const NodeType node_t, const OperationType op_t, const float val)
      : node_tp(node_t), op_tp(op_t), value(val), grad(0) {}

  Impl(const OperationType op_t, const NodeImplSharedPtr &operand1,
       const NodeImplSharedPtr &operand2)
      : node_tp(OPERATION), op_tp(op_t), value(0), grad(0), operand1(operand1),
        operand2(operand2) {}

  static NodeImplSharedPtr constant(const float val) {
    return std::make_shared<Impl>(CONSTANT, UNKNOWN, val);
  }
};

static NodeImplSharedPtr operator+(const NodeImplSharedPtr &lhs,
                                   const NodeImplSharedPtr &rhs);
static NodeImplSharedPtr operator-(const NodeImplSharedPtr &lhs);
static NodeImplSharedPtr operator-(const NodeImplSharedPtr &lhs,
                                   const NodeImplSharedPtr &rhs);
static NodeImplSharedPtr operator*(const NodeImplSharedPtr &lhs,
                                   const NodeImplSharedPtr &rhs);
static NodeImplSharedPtr operator/(const NodeImplSharedPtr &lhs,
                                   const NodeImplSharedPtr &rhs);
static NodeImplSharedPtr pow_impl(const NodeImplSharedPtr &x,
                                  const NodeImplSharedPtr &y);
static NodeImplSharedPtr exp_impl(const NodeImplSharedPtr &x);
static NodeImplSharedPtr log_impl(const NodeImplSharedPtr &x);
static NodeImplSharedPtr sin_impl(const NodeImplSharedPtr &x);
static NodeImplSharedPtr cos_impl(const NodeImplSharedPtr &x);
static NodeImplSharedPtr tan_impl(const NodeImplSharedPtr &x);

static float evaluate_impl(const NodeImpl *node);
static float forward(NodeImpl *node);
static void backward(NodeImpl *node, float grad);
static NodeImplSharedPtr derivative_impl(const NodeImplSharedPtr &node);

// ---

Node::Node(const NodeType node_t, const OperationType op_t, float val) {
  impl = std::make_shared<Impl>(node_t, op_t, val);
}

float Node::value() const { return impl->value; }
float Node::grad() const { return impl->grad; }

Node Node::derivative() const { return Node(derivative_impl(impl)); }
float Node::evaluate() const { return evaluate_impl(impl.get()); }
void Node::numeric_diff() const {
  forward(impl.get());
  backward(impl.get(), 1.f);
}

Node &Node::operator=(const float val) {
  impl->value = val;
  return *this;
}

Node Node::operator-() const { return Node(-impl); }
Node Node::operator+(const Node &rhs) const { return Node(impl + rhs.impl); }
Node Node::operator-(const Node &rhs) const { return Node(impl - rhs.impl); }
Node Node::operator*(const Node &rhs) const { return Node(impl * rhs.impl); }
Node Node::operator/(const Node &rhs) const { return Node(impl / rhs.impl); }
Node exp(const Node &x) { return Node(exp_impl(x.impl)); }
Node log(const Node &x) { return Node(log_impl(x.impl)); }
Node sin(const Node &x) { return Node(sin_impl(x.impl)); }
Node cos(const Node &x) { return Node(cos_impl(x.impl)); }
Node tan(const Node &x) { return Node(tan_impl(x.impl)); }
Node pow(const Node &x, const Node &y) {
  return Node(pow_impl(x.impl, y.impl));
}

// ---

static NodeImplSharedPtr operator+(const NodeImplSharedPtr &lhs,
                                   const NodeImplSharedPtr &rhs) {
  return std::make_shared<NodeImpl>(ADD, lhs, rhs);
}

static NodeImplSharedPtr operator-(const NodeImplSharedPtr &lhs) {
  return std::make_shared<NodeImpl>(NEG, lhs, nullptr);
}

static NodeImplSharedPtr operator-(const NodeImplSharedPtr &lhs,
                                   const NodeImplSharedPtr &rhs) {
  return std::make_shared<NodeImpl>(
      ADD, lhs, std::make_shared<NodeImpl>(NEG, rhs, nullptr));
}

static NodeImplSharedPtr operator*(const NodeImplSharedPtr &lhs,
                                   const NodeImplSharedPtr &rhs) {
  return std::make_shared<NodeImpl>(MUL, lhs, rhs);
}

static NodeImplSharedPtr operator/(const NodeImplSharedPtr &lhs,
                                   const NodeImplSharedPtr &rhs) {
  return std::make_shared<NodeImpl>(
      MUL, lhs, std::make_shared<NodeImpl>(INV, rhs, nullptr));
}

static NodeImplSharedPtr pow_impl(const NodeImplSharedPtr &x,
                                  const NodeImplSharedPtr &y) {
  return std::make_shared<NodeImpl>(POW, x, y);
}

static NodeImplSharedPtr exp_impl(const NodeImplSharedPtr &x) {
  return std::make_shared<NodeImpl>(EXP, x, nullptr);
}

static NodeImplSharedPtr log_impl(const NodeImplSharedPtr &x) {
  return std::make_shared<NodeImpl>(LOG, x, nullptr);
}

static NodeImplSharedPtr sin_impl(const NodeImplSharedPtr &x) {
  return std::make_shared<NodeImpl>(SIN, x, nullptr);
}

static NodeImplSharedPtr cos_impl(const NodeImplSharedPtr &x) {
  return std::make_shared<NodeImpl>(COS, x, nullptr);
}

static NodeImplSharedPtr tan_impl(const NodeImplSharedPtr &x) {
  return std::make_shared<NodeImpl>(TAN, x, nullptr);
}

static float op_eval(const OperationType op_tp, const float x, const float y) {
  switch (op_tp) {
  case ADD:
    return x + y;
  case NEG:
    return -x;
  case MUL:
    return x * y;
  case INV:
    return 1.f / x;
  case POW:
    return std::powf(x, y);
  case EXP:
    return std::expf(x);
  case LOG:
    return std::logf(x);
  case SIN:
    return std::sinf(x);
  case COS:
    return std::cosf(x);
  case TAN:
    return std::tanf(x);
  default:
    return 0.f;
  }
}

static float evaluate_impl(const NodeImpl *node) {
  if (node->node_tp != OPERATION) return node->value;

  const float x = evaluate_impl(node->operand1.get());
  const float y = node->operand2 ? evaluate_impl(node->operand2.get()) : 0.f;
  return op_eval(node->op_tp, x, y);
}

static float forward(NodeImpl *node) {
  node->grad = 0.f;
  if (node->node_tp != OPERATION) return node->value;

  const float x = forward(node->operand1.get());
  const float y = node->operand2 ? forward(node->operand2.get()) : 0.f;
  return node->value = op_eval(node->op_tp, x, y);
}

static inline float square(const float x) { return x * x; }

static void backward(NodeImpl *node, const float grad) {
  if (node->node_tp != OPERATION) {
    node->grad += grad;
    return;
  }

  NodeImpl *x = node->operand1.get();
  NodeImpl *y = node->operand2.get();
  switch (node->op_tp) {
  case ADD:
    backward(x, grad);
    backward(y, grad);
    return;
  case NEG:
    backward(x, -grad);
    return;
  case MUL:
    backward(x, grad * y->value);
    backward(y, grad * x->value);
    return;
  case INV:
    backward(x, -grad / square(x->value));
    return;
  case POW:
    backward(x, grad * node->value * y->value / x->value);
    backward(y, grad * node->value * logf(x->value));
    return;
  case EXP:
    backward(x, grad * node->value);
    return;
  case LOG:
    backward(x, grad / x->value);
    return;
  case SIN:
    backward(x, grad * cosf(x->value));
    return;
  case COS:
    backward(x, -grad * sinf(x->value));
    return;
  case TAN:
    backward(x, grad * (1.f + square(node->value)));
  default:
    break;
  }
}

static NodeImplSharedPtr derivative_impl(const NodeImplSharedPtr &node) {
  if (node->node_tp == CONSTANT) return NodeImpl::constant(0);
  if (node->node_tp == VARIABLE) return NodeImpl::constant(1);

  const NodeImplSharedPtr &x = node->operand1;
  const NodeImplSharedPtr &y = node->operand2;
  const NodeImplSharedPtr dx(x ? derivative_impl(x) : nullptr);
  const NodeImplSharedPtr dy(y ? derivative_impl(y) : nullptr);
  switch (node->op_tp) {
  case ADD:
    return dx + dy;
  case NEG:
    return -dx;
  case MUL:
    return dx * y + x * dy;
  case INV:
    return -dx / pow_impl(x, NodeImpl::constant(2));
  case POW:
    return dx * y * pow_impl(x, y + NodeImpl::constant(-1)) +
           dy * log_impl(x) * pow_impl(x, y);
  case EXP:
    return dx * exp_impl(x);
  case LOG:
    return dx / x;
  case SIN:
    return dx * cos_impl(x);
  case COS:
    return -dx * cos_impl(x);
  case TAN:
    return dx / pow_impl(cos_impl(x), NodeImpl::constant(2));
  default:
    return nullptr;
  }
}

} // namespace mf