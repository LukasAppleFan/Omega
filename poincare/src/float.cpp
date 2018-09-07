#include <poincare/float.h>

namespace Poincare {

template<typename T>
Expression FloatNode<T>::setSign(Sign s, Context & context, Preferences::AngleUnit angleUnit) {
  Expression thisExpr = Number(this);
  Expression result = Float<T>(-m_value);
  thisExpr.replaceWithInPlace(result);
  return result;
}

template<typename T>
int FloatNode<T>::simplificationOrderSameType(const ExpressionNode * e, bool canBeInterrupted) const {
  assert(e->type() == ExpressionNode::Type::Float);
  const FloatNode<T> * other = static_cast<const FloatNode<T> *>(e);
  if (value() < other->value()) {
    return -1;
  }
  if (value() > other->value()) {
    return 1;
  }
  return 0;
}

template<typename T>
Float<T>::Float(T value) : Number(TreePool::sharedPool()->createTreeNode<FloatNode<T>>()) {
  node()->setFloat(value);
}

template class FloatNode<float>;
template class FloatNode<double>;

template Float<float>::Float(float value);
template Float<double>::Float(double value);

}
