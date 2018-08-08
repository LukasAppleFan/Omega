#include <poincare/matrix_inverse.h>
#include <poincare/matrix.h>
#include <poincare/division.h>
#include <poincare/undefined.h>
#include <poincare/power.h>
extern "C" {
#include <assert.h>
}
#include <cmath>

namespace Poincare {

Expression::Type MatrixInverse::type() const {
  return Type::MatrixInverse;
}

Expression * MatrixInverse::clone() const {
  MatrixInverse * a = new MatrixInverse(m_operands, true);
  return a;
}

Expression MatrixInverse::shallowReduce(Context& context, Preferences::AngleUnit angleUnit) {
  Expression * e = Expression::shallowReduce(context, angleUnit);
  if (e != this) {
    return e;
  }
  Expression * op = editableOperand(0);
#if MATRIX_EXACT_REDUCING
  if (!op->recursivelyMatches(Expression::IsMatrix)) {
    detachOperand(op);
    return replaceWith(new Power(op, new Rational(-1), false), true)->shallowReduce(context, angleUnit);
  }
  if (op->type() == Type::Matrix) {
    Matrix * mat = static_cast<Matrix *>(op);
    if (mat->numberOfRows() != mat->numberOfColumns()) {
      return replaceWith(new Undefined(), true);
    }
  }
  return this;
#else
  detachOperand(op);
  return replaceWith(new Power(op, new Rational(-1), false), true)->shallowReduce(context, angleUnit);
#endif
}

// TODO: handle this exactly in shallowReduce for small dimensions.
template<typename T>
Evaluation<T> MatrixInverse::templatedApproximate(Context& context, Preferences::AngleUnit angleUnit) const {
  Evaluation<T> * input = operand(0)->privateApproximate(T(), context, angleUnit);
  Evaluation<T> * inverse = input->createInverse();
  if (inverse == nullptr) {
    inverse = new Complex<T>(Complex<T>::Undefined());
  }
  delete input;
  return inverse;
}

}

