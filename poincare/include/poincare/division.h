#ifndef POINCARE_DIVISION_H
#define POINCARE_DIVISION_H

#include <poincare/expression.h>
#include <poincare/approximation_helper.h>
#include <poincare/serialization_helper.h>

namespace Poincare {

class Division;

class DivisionNode : public ExpressionNode {
public:
  // TreeNode
  static DivisionNode * FailedAllocationStaticNode();
  size_t size() const override { return sizeof(DivisionNode); }
#if TREE_LOG
  const char * description() const override { return "Division";  }
#endif
  int numberOfChildren() const override { return 2; }

  // Properties
  Type type() const override { return Type::Division; }
  int polynomialDegree(char symbolName) const override;

  // Approximation
  virtual Evaluation<float> approximate(SinglePrecision p, Context& context, Preferences::AngleUnit angleUnit) const override {
    return ApproximationHelper::MapReduce<float>(
        this, context, angleUnit, compute<float>,
        computeOnComplexAndMatrix<float>, computeOnMatrixAndComplex<float>,
        computeOnMatrices<float>);
  }
  virtual Evaluation<double> approximate(DoublePrecision p, Context& context, Preferences::AngleUnit angleUnit) const override {
    return ApproximationHelper::MapReduce<double>(
        this, context, angleUnit, compute<double>,
        computeOnComplexAndMatrix<double>, computeOnMatrixAndComplex<double>,
        computeOnMatrices<double>);
  }

  // Layout
  virtual bool needsParenthesesWithParent(const SerializationHelperInterface * parent) const;
  LayoutRef createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;
  int serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override {
    return SerializationHelper::Infix(this, buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits, "/");
  }

  // Simplification
  virtual Expression shallowReduce(Context& context, Preferences::AngleUnit angleUnit) const override;

private:
  template<typename T> static Complex<T> compute(const std::complex<T> c, const std::complex<T> d);
  template<typename T> static MatrixComplex<T> computeOnMatrixAndComplex(const MatrixComplex<T> m, const std::complex<T> c) {
    return ApproximationHelper::ElementWiseOnMatrixComplexAndComplex(m, c, compute<T>);
  }
  template<typename T> static MatrixComplex<T> computeOnComplexAndMatrix(const std::complex<T> c, const MatrixComplex<T> n);
  template<typename T> static MatrixComplex<T> computeOnMatrices(const MatrixComplex<T> m, const MatrixComplex<T> n);
};

class Division : public Expression {
public:
  Division(Expression numerator, Expression denominator) {
    addChild(numberator);
    addChild(denominator);
  }
  Division(const DivisionNode * n) : Expression(n) {}
  Expression shallowReduce(Context & context, Preferences::AngleUnit angleUnit);
};

}

#endif
