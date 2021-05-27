#ifndef POINCARE_POWER_H
#define POINCARE_POWER_H

#include <poincare/approximation_helper.h>
#include <poincare/multiplication.h>
#include <poincare/rational.h>

namespace Poincare {

class Power;

class PowerNode final : public ExpressionNode {
public:

  // TreeNode
  size_t size() const override { return sizeof(PowerNode); }
  int numberOfChildren() const override { return 2; }
#if POINCARE_TREE_LOG
  void logNodeName(std::ostream & stream) const override {
    stream << "Power";
  }
#endif

  // Complex
  bool isReal(Context * context) const;

  // Properties
  Type type() const override { return Type::Power; }
  Sign sign(Context * context) const override;
  NullStatus nullStatus(Context * context) const override {
    // NonNull Status can't be returned because base could be infinite.
    return childAtIndex(0)->nullStatus(context) == NullStatus::Null ?  NullStatus::Null : NullStatus::Unknown;
  }
  Expression setSign(Sign s, ReductionContext reductionContext) override;
  bool childAtIndexNeedsUserParentheses(const Expression & child, int childIndex) const override;
  Expression removeUnit(Expression * unit) override;

  int polynomialDegree(Context * context, const char * symbolName) const override;
  int getPolynomialCoefficients(Context * context, const char * symbolName, Expression coefficients[]) const override;

  template<typename T> static Complex<T> computeNotPrincipalRealRootOfRationalPow(const std::complex<T> c, T p, T q);
  template<typename T> static Complex<T> compute(const std::complex<T> c, const std::complex<T> d, Preferences::ComplexFormat complexFormat);

private:
  constexpr static int k_maxApproximatePowerMatrix = 1000;

  // Layout
  Layout createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;

  // Serialize
  bool childNeedsSystemParenthesesAtSerialization(const TreeNode * child) const override;
  int serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;

  // Simplify
  Expression shallowReduce(ReductionContext reductionContext) override;
  Expression shallowBeautify(ReductionContext * reductionContext) override;
  LayoutShape leftLayoutShape() const override { return childAtIndex(0)->leftLayoutShape(); }
  LayoutShape rightLayoutShape() const override { return LayoutShape::RightOfPower; }
  int simplificationOrderGreaterType(const ExpressionNode * e, bool ascending, bool canBeInterrupted, bool ignoreParentheses) const override;
  int simplificationOrderSameType(const ExpressionNode * e, bool ascending, bool canBeInterrupted, bool ignoreParentheses) const override;
  Expression denominator(ReductionContext reductionContext) const override;
  bool derivate(ReductionContext reductionContext, Expression symbol, Expression symbolValue) override;
  // Evaluation
  template<typename T> static MatrixComplex<T> computeOnComplexAndMatrix(const std::complex<T> c, const MatrixComplex<T> n, Preferences::ComplexFormat complexFormat);
  template<typename T> static MatrixComplex<T> computeOnMatrixAndComplex(const MatrixComplex<T> m, const std::complex<T> d, Preferences::ComplexFormat complexFormat);
  template<typename T> static MatrixComplex<T> computeOnMatrices(const MatrixComplex<T> m, const MatrixComplex<T> n, Preferences::ComplexFormat complexFormat);
  Evaluation<float> approximate(SinglePrecision p, ApproximationContext approximationContext) const override {
    return templatedApproximate<float>(approximationContext);
  }
  Evaluation<double> approximate(DoublePrecision p, ApproximationContext approximationContext) const override {
    return templatedApproximate<double>(approximationContext);
  }
 template<typename T> Evaluation<T> templatedApproximate(ApproximationContext approximationContext) const;
};

class Power final : public Expression {
  friend class PowerNode;
  friend class Round;
public:
  Power(const PowerNode * n) : Expression(n) {}
  static Power Builder(Expression base, Expression exponent) { return TreeHandle::FixedArityBuilder<Power, PowerNode>({base, exponent}); }

  Expression setSign(ExpressionNode::Sign s, ExpressionNode::ReductionContext reductionContext);
  int getPolynomialCoefficients(Context * context, const char * symbolName, Expression coefficients[]) const;
  Expression shallowReduce(ExpressionNode::ReductionContext reductionContext);
  Expression shallowBeautify(ExpressionNode::ReductionContext * reductionContext);
  bool derivate(ExpressionNode::ReductionContext reductionContext, Expression symbol, Expression symbolValue);

private:
  constexpr static int k_maxExactPowerMatrix = 100;
  constexpr static int k_maxNumberOfTermsInExpandedMultinome = 25;

  // Simplification
  static Expression PowerRationalRational(Rational base, Rational index, ExpressionNode::ReductionContext reductionContext);
  static Expression PowerIntegerRational(Integer base, Rational index, ExpressionNode::ReductionContext reductionContext);
  static Expression CreateComplexExponent(const Expression & r, ExpressionNode::ReductionContext reductionContext); // Returns e^(i*pi*r)
  static bool RationalExponentShouldNotBeReduced(const Rational & b, const Rational & r);
  bool isLogarithmOfSameBase(Expression e) const;
  bool isNthRootOfUnity() const;

  // Unit
  Expression removeUnit(Expression * unit);

  Expression denominator(ExpressionNode::ReductionContext reductionContext) const;
};

}

#endif
