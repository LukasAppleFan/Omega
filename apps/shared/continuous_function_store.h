#ifndef SHARED_CONTINUOUS_FUNCTION_STORE_H
#define SHARED_CONTINUOUS_FUNCTION_STORE_H

#include "expression_model_store.h"
#include "continuous_function.h"
#include <stdint.h>

namespace Shared {

// ContinuousFunctionStore stores functions and gives them a color.

class ContinuousFunctionStore : public ExpressionModelStore {
public:
  ContinuousFunctionStore() : ExpressionModelStore() {}
  uint32_t storeChecksum();
  int numberOfActiveFunctions() const {
    return numberOfModelsSatisfyingTest(&isFunctionActive, nullptr);
  }
  Ion::Storage::Record activeRecordAtIndex(int i) const {
    return recordSatisfyingTestAtIndex(i, &isFunctionActive, nullptr);
  }
  // TODO Hugo
  bool displaysNonCartesianFunctions(int * nbActiveFunctions = nullptr) const;
  int numberOfActiveFunctionsOfType(ContinuousFunction::PlotType plotType) const {
    return numberOfModelsSatisfyingTest(&isFunctionActiveOfType, &plotType);
  }
  Ion::Storage::Record activeRecordOfTypeAtIndex(ContinuousFunction::PlotType plotType, int i) const {
    return recordSatisfyingTestAtIndex(i, &isFunctionActiveOfType, &plotType);
  }
  ExpiringPointer<ContinuousFunction> modelForRecord(Ion::Storage::Record record) const { return ExpiringPointer<ContinuousFunction>(static_cast<ContinuousFunction *>(privateModelForRecord(record))); }
  // TODO Hugo : Handle cache
  // ContinuousFunctionCache * cacheAtIndex(int i) const { return (i < ContinuousFunctionCache::k_numberOfAvailableCaches) ? m_functionCaches + i : nullptr; }
  Ion::Storage::Record::ErrorStatus addEmptyModel() override;

private:
  static bool isFunctionActive(ExpressionModelHandle * model, void * context) {
    // An active function must be defined
    return isModelDefined(model, context) && static_cast<ContinuousFunction *>(model)->isActive();
  }
  // TODO Hugo : Factorize or delete function_store.cpp, simplify methods
  const char * modelExtension() const override { return Ion::Storage::funcExtension; }
  ExpressionModelHandle * setMemoizedModelAtIndex(int cacheIndex, Ion::Storage::Record record) const override;
  ExpressionModelHandle * memoizedModelAtIndex(int cacheIndex) const override;
  static bool isFunctionActiveOfType(ExpressionModelHandle * model, void * context) {
    ContinuousFunction::PlotType plotType = *static_cast<ContinuousFunction::PlotType *>(context);
    return isFunctionActive(model, context) && plotType == static_cast<ContinuousFunction *>(model)->plotType();
  }
  mutable ContinuousFunction m_functions[k_maxNumberOfMemoizedModels];
  // TODO Hugo : Handle cache
  // mutable ContinuousFunctionCache m_functionCaches[ContinuousFunctionCache::k_numberOfAvailableCaches];
};

}

#endif
