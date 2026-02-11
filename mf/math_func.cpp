#include "math_func.hpp"
#include <cmath>
#include <queue>
#include <vector>

namespace mf {
using NodeImpl = Node::Impl;
using NodeImplSharedPtr = std::shared_ptr<Node::Impl>;
using TopoOrder = std::vector<NodeImpl *>;

struct Node::Impl {
  NodeType type;
  OperationType op;
  float value, grad;

  bool visited;
  int size;                       // count of self and sub-nodes
  NodeImplSharedPtr opnd1, opnd2; // operand
  NodeImplSharedPtr deriv;

  Impl(const NodeType node_t, const OperationType op_t, const float val)
      : type(node_t), op(op_t), value(val), grad(0), visited(false), size(1) {}

  Impl(const OperationType op_t, NodeImplSharedPtr operand1,
       NodeImplSharedPtr operand2)
      : type(OPERATION), op(op_t), value(0), grad(0), visited(false),
        opnd1(std::move(operand1)), opnd2(std::move(operand2)) {
    size = 1 + opnd1->size;
    if (opnd2) size += opnd2->size;
  }

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

static TopoOrder reverse_topo_sort(const NodeImplSharedPtr &expr);
static float forward_impl(const TopoOrder &order);
static void backward_impl(const TopoOrder &order);
static NodeImplSharedPtr derivative_impl(const TopoOrder &order);

// --- Node ---

Node::Node(const NodeType node_t, const OperationType op_t, float val) {
  impl = std::make_shared<Impl>(node_t, op_t, val);
}

float Node::value() const { return impl->value; }
float Node::grad() const { return impl->grad; }

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

// --- Function ---

float Function::forward() {
  if (reverse_topo.empty()) reverse_topo = reverse_topo_sort(expr.impl);
  return forward_impl(reverse_topo);
}

void Function::backward() const {
  if (reverse_topo.empty()) return;
  backward_impl(reverse_topo);
}

Function Function::derivative() {
  if (reverse_topo.empty()) reverse_topo = reverse_topo_sort(expr.impl);
  return Node(derivative_impl(reverse_topo));
}

// --- end ---

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

static void dfs(TopoOrder &order, NodeImpl *node) {
  if (!node || node->visited) return;
  node->visited = true;
  dfs(order, node->opnd1.get());
  dfs(order, node->opnd2.get());
  order.push_back(node);
}

static TopoOrder reverse_topo_sort(const NodeImplSharedPtr &expr) {
  TopoOrder order;
  order.reserve(expr->size);
  dfs(order, expr.get());

  for (NodeImpl *node : order) node->visited = false;
  return order;
}

static float node_eval(const NodeImpl *node) {
  if (node->type != OPERATION) return node->value;

  const float x = node->opnd1->value;
  const float y = node->opnd2 ? node->opnd2->value : 0.f;
  switch (node->op) {
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

static float forward_impl(const TopoOrder &order) {
  for (auto it = order.begin(); it != order.end(); ++it) {
    NodeImpl *node = *it;
    node->value = node_eval(node);
  }
  return order.back()->value;
}

static inline float square(const float x) { return x * x; }

static void backward_impl(const TopoOrder &order) {
  for (NodeImpl *node : order) node->grad = 0.f;

  order.back()->grad = 1.f;
  for (auto it = order.rbegin(); it != order.rend(); ++it) {
    const NodeImpl *node = *it;
    if (node->type == OPERATION) {
      const float grad = node->grad;
      const NodeImplSharedPtr &x = node->opnd1;
      const NodeImplSharedPtr &y = node->opnd2;
      switch (node->op) {
      case ADD:
        x->grad += grad;
        y->grad += grad;
        break;
      case NEG:
        x->grad -= grad;
        break;
      case MUL:
        x->grad += grad * y->value;
        y->grad += grad * x->value;
        break;
      case INV:
        x->grad -= grad * square(node->value);
        break;
      case POW:
        x->grad += grad * node->value * y->value / x->value;
        y->grad += grad * node->value * logf(x->value);
        break;
      case EXP:
        x->grad += grad * node->value;
        break;
      case LOG:
        x->grad += grad / x->value;
        break;
      case SIN:
        x->grad += grad * std::cosf(x->value);
        break;
      case COS:
        x->grad -= grad * std::sinf(x->value);
        break;
      case TAN:
        x->grad += grad * (1.f + square(node->value));
        break;
      default:
        break;
      }
    }
  }
}

static NodeImplSharedPtr node_deriv(const NodeImpl *node) {
  if (node->type == CONSTANT) return NodeImpl::constant(0);
  if (node->type == VARIABLE) return NodeImpl::constant(1);

  const NodeImplSharedPtr &x = node->opnd1;
  const NodeImplSharedPtr &y = node->opnd2;
  const NodeImplSharedPtr dx = x->deriv;
  const NodeImplSharedPtr dy(y ? y->deriv : nullptr);
  switch (node->op) {
  case ADD:
    return dx + dy;
  case NEG:
    return -dx;
  case MUL:
    return dx * y + x * dy;
  case INV:
    return -dx / pow_impl(x, NodeImpl::constant(2));
  case POW:
    return (dx * y + dy * x * log_impl(x)) *
           pow_impl(x, y - NodeImpl::constant(1));
  case EXP:
    return dx * exp_impl(x);
  case LOG:
    return dx / x;
  case SIN:
    return dx * cos_impl(x);
  case COS:
    return -dx * sin_impl(x);
  case TAN:
    return dx / pow_impl(cos_impl(x), NodeImpl::constant(2));
  default:
    return nullptr;
  }
}

static NodeImplSharedPtr derivative_impl(const TopoOrder &order) {
  for (auto it = order.begin(); it != order.end(); ++it) {
    NodeImpl *node = *it;
    node->deriv = node_deriv(node);
  }

  const NodeImplSharedPtr deriv = order.back()->deriv;
  for (NodeImpl *node : order) node->deriv.reset();
  return deriv;
}

} // namespace mf