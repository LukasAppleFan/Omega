#ifndef SHARED_CONTINUOUS_FUNCTION_H
#define SHARED_CONTINUOUS_FUNCTION_H

#include "function.h"
#include "range_1D.h"
#include <apps/i18n.h>
#include <poincare/symbol_abstract.h>
#include <poincare/conic.h>
#include <poincare/preferences.h>
#include "continuous_function_cache.h"

namespace Shared {

class ContinuousFunction : public Function {
  /* We want the cache to be able to call privateEvaluateXYAtParameter to
   * bypass cache lookup when memoizing the function's values. */
  friend class ContinuousFunctionCache;
public:
  /* ContinuousFunction */

  // Create a record with baseName
  static ContinuousFunction NewModel(Ion::Storage::Record::ErrorStatus * error, const char * baseName = nullptr);
  // Builder
  ContinuousFunction(Ion::Storage::Record record = Record()) : Function(record), m_cache(nullptr) {}

  /* Symbol and Plot types */

  static constexpr size_t k_numberOfSymbolTypes = 3;
  enum class SymbolType : uint8_t {
    Theta = 0,
    T,
    X,
  };

  static constexpr size_t k_numberOfPlotTypes = 14;
  enum class PlotType : uint8_t {
    Polar = 0,
    Parametric,
    // All following types use x as a symbol
    Cartesian,
    Line,
    HorizontalLine,
    // All following types shall never be active in values table
    VerticalLine,
    Inequation,
    Circle,
    Ellipse,
    Parabola,
    Hyperbola,
    Other,
    // All following types shall never be active
    Undefined,
    Unhandled
  };

  static_assert(
    (static_cast<SymbolType>(PlotType::Polar) == SymbolType::Theta &&
      static_cast<SymbolType>(PlotType::Parametric) == SymbolType::T &&
      static_cast<SymbolType>(PlotType::Cartesian) == SymbolType::X),
    "First PlotTypes should match SymbolTypes");
  // Return Message corresponding to SymbolType
  static I18n::Message MessageForSymbolType(SymbolType symbolType);
  // Return ContinuousFunction's PlotType
  PlotType plotType() const { return recordData()->plotType(); }
  // Return ContinuousFunction's SymbolType
  SymbolType symbolType() const;
  // Return ContinuousFunction's PlotType message
  I18n::Message plotTypeMessage() const;

  /* Function */

  // Return Message corresponding to ContinuousFunction's SymbolType
  I18n::Message parameterMessageName() const override { return MessageForSymbolType(symbolType()); }
  // Return CodePoint corresponding to ContinuousFunction's SymbolType
  CodePoint symbol() const override;
  // Insert ContinuousFunction's name and argument in buffer ("f(x)" or "y")
  int nameWithArgument(char * buffer, size_t bufferSize) override;
  // Insert x or y (depending on index) value in buffer
  int printValue(int index, double cursorT, double cursorX, double cursorY, char * buffer, int bufferSize, int precision, Poincare::Context * context) override;

  /* ExpressionModel */

  Ion::Storage::Record::ErrorStatus setContent(const char * c, Poincare::Context * context) override;

  /* Properties */

  // Wether to draw a dotted or solid line (Strict inequalities).
  bool drawDottedCurve() const;
  // Wether to draw the area "below" the curve or not
  bool drawInferiorArea() const;
  // Wether to draw the area "above" the curve or not
  bool drawSuperiorArea() const;
  // If the ContinuousFunction should be plot with two curves
  bool hasTwoCurves() const override { return m_model.hasTwoCurves(); }
  // If the ContinuousFunction should be considered active in table
  bool isActiveInTable() const { return plotType() < PlotType::VerticalLine && isActive(); }
  // If the ContinuousFunction has x for unknown symbol
  bool isAlongX() const { return symbol() == 'x'; }
  // If the ContinuousFunction is a conic
  bool isConic() const { return plotType() >= PlotType::Circle && plotType() <= PlotType::Hyperbola; }
  // If the ContinuousFunction is named ("f(x)=...")
  bool isNamed() const;
  // Compute line parameters (slope and intercept) from ContinuousFunction
  void getLineParameters(double * slope, double * intercept, Poincare::Context * context) const;
  // Compute conic parameters from ContinuousFunction
  Poincare::Conic getConicParameters(Poincare::Context * context) const;

  /* Expression */

  // Return the unaltered equation expression, replacing unknown symbols.
  Poincare::Expression originalEquation(const Ion::Storage::Record * record) const {
    return m_model.originalEquation(record, symbol());
  }
  // Update plotType as well as tMin and tMax values.
  void udpateModel(Poincare::Context * context);

  /* Evaluation */

  Poincare::Coordinate2D<double> evaluate2DAtParameter(double t, Poincare::Context * context, int curveIndex = 0) const {
    return templatedApproximateAtParameter(t, context, curveIndex);
  }
  Poincare::Coordinate2D<float> evaluateXYAtParameter(float t, Poincare::Context * context, int curveIndex = 0) const override {
    return m_cache ? m_cache->valueForParameter(this, context, t, curveIndex) : privateEvaluateXYAtParameter<float>(t, context, curveIndex);
  }
  Poincare::Coordinate2D<double> evaluateXYAtParameter(double t, Poincare::Context * context, int curveIndex = 0) const override {
    return privateEvaluateXYAtParameter<double>(t, context, curveIndex);
  }

  /* Derivative */

  // If derivative should be displayed
  bool displayDerivative() const { return recordData()->displayDerivative(); }
  // Set derivative display status
  void setDisplayDerivative(bool display) { return recordData()->setDisplayDerivative(display); }
  // Insert derivative name with argument in buffer (f'(x) or y')
  int derivativeNameWithArgument(char * buffer, size_t bufferSize);
  // Approximate derivative at x, on given secondary curve if there is one
  double approximateDerivative(double x, Poincare::Context * context, int secondaryCurveIndex = 0) const;

  /* tMin and tMax */

  // tMin getter
  float tMin() const override { return recordData()->tMin(); }
  // tMax getter
  float tMax() const override { return recordData()->tMax(); }
  // tMin setter
  void setTMin(float tMin);
  // tMax setter
  void setTMax(float tMax);

  /* Range */

  // If curve expression is based on costly algorithm for approximation
  bool basedOnCostlyAlgorithms(Poincare::Context * context) const override;
  // Set x range for display adapt y ranges accordingly as well.
  void xRangeForDisplay(float xMinLimit, float xMaxLimit, float * xMin, float * xMax, float * yMinIntrinsic, float * yMaxIntrinsic, Poincare::Context * context) const override;
  // Set y range for display
  void yRangeForDisplay(float xMin, float xMax, float yMinForced, float yMaxForced, float ratio, float * yMin, float * yMax, Poincare::Context * context, bool optimizeRange) const override;

  /* Extremum */

  // Compute coordinates of the next minimun, from a starting point
  Poincare::Coordinate2D<double> nextMinimumFrom(double start, double max, Poincare::Context * context, double relativePrecision, double minimalStep, double maximalStep) const;
  // Compute coordinates of the next maximum, from a starting point
  Poincare::Coordinate2D<double> nextMaximumFrom(double start, double max, Poincare::Context * context, double relativePrecision, double minimalStep, double maximalStep) const;

  /* Roots */

  // Compute coordinates of the next root, from a starting point
  Poincare::Coordinate2D<double> nextRootFrom(double start, double max, Poincare::Context * context, double relativePrecision, double minimalStep, double maximalStep) const;
  // Compute coordinates of the next intersection, from a starting point
  Poincare::Coordinate2D<double> nextIntersectionFrom(double start, double max, Poincare::Context * context, Poincare::Expression e, double relativePrecision, double minimalStep, double maximalStep, double eDomainMin = -INFINITY, double eDomainMax = INFINITY) const;

  /* Integral */

  // Return the expression of the integral between start and end
  Poincare::Expression sumBetweenBounds(double start, double end, Poincare::Context * context) const override;

  /* Cache */

  ContinuousFunctionCache * cache() const { return m_cache; }
  void setCache(ContinuousFunctionCache * v) { m_cache = v; }
  void didBecomeInactive() override { m_cache = nullptr; }
  static constexpr char k_unnamedRecordFirstChar = '?';
private:
  static constexpr char k_unknownName[2] = {UCodePointUnknown, 0};
  static constexpr char k_ordinateName[2] = "y";
  static constexpr float k_polarParamRangeSearchNumberOfPoints = 100.0f; // This is ad hoc, no special justification
  static constexpr Poincare::Preferences::UnitFormat k_defaultUnitFormat = Poincare::Preferences::UnitFormat::Metric;
  static constexpr Poincare::ExpressionNode::SymbolicComputation k_defaultSymbolicComputation = Poincare::ExpressionNode::SymbolicComputation::DoNotReplaceAnySymbol;

  /* Range */

  // Return true if the function will not be displayed if t is outside x range.
  bool shouldClipTRangeToXRange() const override { return isAlongX(); } // TODO Hugo : Delete this method if unused
  // Return step computed from t range or NAN if isAlongX() is true.
  float rangeStep() const override;

  /* Expressions */

  // Return the expression representing the equation for computations
  Poincare::Expression expressionEquation(Poincare::Context * context) const  {
    return m_model.expressionEquation(this, context);
  }
  // Return reduced curve expression derivative
  Poincare::Expression expressionDerivateReduced(Poincare::Context * context) const { return m_model.expressionDerivateReduced(this, context); }

  /* Properties */

  // Update ContinuousFunction's PlotType
  void updatePlotType(Poincare::Context * context);
  // Return the equation's symbol
  Poincare::ExpressionNode::Type equationSymbol() const { return recordData()->equationSymbol(); }
  // If y coefficient at yDeg in equation is NonNull. Can also compute its sign.
  bool isYCoefficientNonNull(Poincare::Expression equation, int yDeg, Poincare::Context * context, Poincare::ExpressionNode::Sign * YSign = nullptr) const;

  /* Evaluation */

  typedef Poincare::Coordinate2D<double> (*ComputePointOfInterest)(Poincare::Expression e, const char * symbol, double start, double max, Poincare::Context * context, double relativePrecision, double minimalStep, double maximalStep);
  // Compute coordinates of the next point of interest, from a starting point
  Poincare::Coordinate2D<double> nextPointOfInterestFrom(double start, double max, Poincare::Context * context, ComputePointOfInterest compute, double relativePrecision, double minimalStep, double maximalStep) const;
  // Evaluate XY at parameter (distinct from approximation with Polar types)
  template<typename T> Poincare::Coordinate2D<T> privateEvaluateXYAtParameter(T t, Poincare::Context * context, int secondaryCurveIndex = 0) const;
  // Approximate XY at parameter
  template<typename T> Poincare::Coordinate2D<T> templatedApproximateAtParameter(T t, Poincare::Context * context, int secondaryCurveIndex = 0) const;

  /* Record */

  // TODO Hugo : Padding
  class __attribute__((packed)) RecordDataBuffer : public Shared::Function::RecordDataBuffer {
  public:
    RecordDataBuffer(KDColor color) :
        Shared::Function::RecordDataBuffer(color),
        m_domain(-INFINITY, INFINITY),
        m_plotType(PlotType::Undefined),
        m_equationSymbol(Poincare::ExpressionNode::Type::Equal),
        m_displayDerivative(false) {}
    PlotType plotType() const { return m_plotType; }
    void setPlotType(PlotType plotType) { m_plotType = plotType; }
    bool displayDerivative() const { return m_displayDerivative; }
    void setDisplayDerivative(bool display) { m_displayDerivative = display; }
    Poincare::ExpressionNode::Type equationSymbol() const { return m_equationSymbol; }
    void setEquationSymbol(Poincare::ExpressionNode::Type equationSymbol) { m_equationSymbol = equationSymbol; }
    bool isActive() const override { return Shared::Function::RecordDataBuffer::isActive() && m_plotType != PlotType::Unhandled && m_plotType != PlotType::Undefined; }
    float tMin() const { return m_domain.min(); }
    float tMax() const { return m_domain.max(); }
    void setTMin(float tMin) { m_domain.setMin(tMin); }
    void setTMax(float tMax) { m_domain.setMax(tMax); }
  private:
    Range1D m_domain;
    PlotType m_plotType;
    Poincare::ExpressionNode::Type m_equationSymbol;
    bool m_displayDerivative;
    /* In the record, after the boolean flag about displayDerivative, there is
     * the expression of the function, directly copied from the pool. */
    //char m_expression[0];
  };

  // Return metadata size
  size_t metaDataSize() const override { return sizeof(RecordDataBuffer); }
  // Return record data
  RecordDataBuffer * recordData() const {
    assert(!isNull());
    return reinterpret_cast<RecordDataBuffer *>(const_cast<void *>(value().buffer));
  }

  /* Model */

  class Model : public ExpressionModel {
  public:
    Model() :
        ExpressionModel(),
        m_hasTwoCurves(false),
        m_equationSymbol(Poincare::ExpressionNode::Type::Equal),
        m_plotType(PlotType::Undefined),
        m_expressionDerivate() {}
    // Return the expression to plot.
    Poincare::Expression expressionReduced(const Ion::Storage::Record * record, Poincare::Context * context) const override;
    // Return the expression of the named function (right side of the equal)
    Poincare::Expression expressionClone(const Ion::Storage::Record * record) const override;
    // Return the entire expression that the user inputted. Replace symbols.
    Poincare::Expression originalEquation(const Ion::Storage::Record * record, CodePoint symbol) const;
    // Return the expression representing the equation
    Poincare::Expression expressionEquation(const Ion::Storage::Record * record, Poincare::Context * context) const;
    // Return the derivative of the expression to plot.
    Poincare::Expression expressionDerivateReduced(const Ion::Storage::Record * record, Poincare::Context * context) const;
    // Rename the record if needed. Record pointer might get corrupted.
    Ion::Storage::Record::ErrorStatus renameRecordIfNeeded(Ion::Storage::Record * record, const char * c, Poincare::Context * context, CodePoint symbol);
    // Build the expression from text, handling f(x)=... cartesian equations
    Poincare::Expression buildExpressionFromText(const char * c, CodePoint symbol = 0, Poincare::Context * context = nullptr) const override;
    // Tidy the model
    void tidy() const override;
    // m_hasTwoCurves getter
    bool hasTwoCurves() const { return m_hasTwoCurves; }
    // m_equationSymbol getter
    Poincare::ExpressionNode::Type equationSymbol() const { return m_equationSymbol; }
    // m_equationSymbol setter
    void setEquationSymbol(Poincare::ExpressionNode::Type equationSymbol) const { m_equationSymbol = equationSymbol; }
    // m_plotType getter
    PlotType plotType() const { return m_plotType; }
  private:
    // Return address of the record's expression
    void * expressionAddress(const Ion::Storage::Record * record) const override;
    // Return size of the record's expression
    size_t expressionSize(const Ion::Storage::Record * record) const override;
    mutable bool m_hasTwoCurves;
    mutable Poincare::ExpressionNode::Type m_equationSymbol;
    // TODO Hugo : Improve this m_plotType workaround
    mutable PlotType m_plotType;
    mutable Poincare::Expression m_expressionDerivate;
  };

  // Return model pointer
  const ExpressionModel * model() const override { return &m_model; }

  Model m_model;
  ContinuousFunctionCache * m_cache;
};

}

#endif
