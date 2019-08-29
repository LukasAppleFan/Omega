#include "values_controller.h"
#include <assert.h>
#include "../../constant.h"
#include "../app.h"

using namespace Shared;
using namespace Poincare;

namespace Graph {

ValuesController::ValuesController(Responder * parentResponder, InputEventHandlerDelegate * inputEventHandlerDelegate, Interval * interval, ButtonRowController * header) :
  Shared::ValuesController(parentResponder, inputEventHandlerDelegate, header, I18n::Message::X, &m_intervalParameterController, interval),
  m_functionTitleCells{},
  m_floatCells{},
  m_functionParameterController(this),
  m_intervalParameterController(this, inputEventHandlerDelegate, m_interval),
  m_derivativeParameterController(this)
{
  for (int i = 0; i < k_maxNumberOfFunctions; i++) {
    m_functionTitleCells[i].setOrientation(FunctionTitleCell::Orientation::HorizontalIndicator);
    m_functionTitleCells[i].setFont(KDFont::SmallFont);
  }
}

void ValuesController::willDisplayCellAtLocation(HighlightCell * cell, int i, int j) {
  Shared::ValuesController::willDisplayCellAtLocation(cell, i, j);
  // The cell is the abscissa title cell:
  if (typeAtLocation(i,j) == 0) {
    EvenOddMessageTextCell * mytitleCell = (EvenOddMessageTextCell *)cell;
    mytitleCell->setMessage(I18n::Message::X);
    return;
  }
  // The cell is a function title cell:
  if (typeAtLocation(i,j) == 1) {
    Shared::BufferFunctionTitleCell * myFunctionCell = (Shared::BufferFunctionTitleCell *)cell;
    const size_t bufferNameSize = Shared::Function::k_maxNameWithArgumentSize + 1;
    char bufferName[bufferNameSize];
    bool isDerivative = isDerivativeColumn(i);
    /* isDerivativeColumn uses expiring pointers, so "function" must be created
     * after the isDerivativeColumn call, else it will expire. */
    Shared::ExpiringPointer<CartesianFunction> function = functionStore()->modelForRecord(recordAtColumn(i));
    if (isDerivative) {
      function->derivativeNameWithArgument(bufferName, bufferNameSize);
    } else {
      function->nameWithArgument(bufferName, bufferNameSize);
    }
    myFunctionCell->setText(bufferName);
    myFunctionCell->setColor(function->color());
  }
}

I18n::Message ValuesController::emptyMessage() {
  if (functionStore()->numberOfDefinedModels() == 0) {
    return I18n::Message::NoFunction;
  }
  return I18n::Message::NoActivatedFunction;
}

IntervalParameterController * ValuesController::intervalParameterController() {
  return &m_intervalParameterController;
}

Ion::Storage::Record ValuesController::recordAtColumn(int i) {
  assert(typeAtLocation(i, 0) == 1);
  int index = 1;
  for (int k = 0; k < functionStore()->numberOfDefinedModels(); k++) {
    Ion::Storage::Record record = functionStore()->definedRecordAtIndex(k);
    ExpiringPointer<CartesianFunction> f = functionStore()->modelForRecord(record);
    if (f->isActive()) {
      if (i == index) {
        return record;
      }
      index++;
      if (f->displayDerivative()) {
        if (i == index) {
          return record;
        }
        index++;
      }
    }
  }
  assert(false);
  return nullptr;
}

bool ValuesController::isDerivativeColumn(int i) {
  assert(typeAtLocation(i, 0) == 1);
  int index = 1;
  for (int k = 0; k < functionStore()->numberOfDefinedModels(); k++) {
    ExpiringPointer<CartesianFunction> f = functionStore()->modelForRecord(functionStore()->definedRecordAtIndex(k));
    if (f->isActive()) {
      if (i == index) {
        return false;
      }
      index++;
      if (f->displayDerivative()) {
        if (i == index) {
          return true;
        }
        index++;
      }
    }
  }
  assert(false);
  return false;
}

int ValuesController::maxNumberOfCells() {
  return k_maxNumberOfCells;
}

int ValuesController::maxNumberOfFunctions() {
  return k_maxNumberOfFunctions;
}

Shared::BufferFunctionTitleCell * ValuesController::functionTitleCells(int j) {
  assert(j >= 0 && j < k_maxNumberOfFunctions);
  return &m_functionTitleCells[j];
}

EvenOddBufferTextCell * ValuesController::floatCells(int j) {
  assert(j >= 0 && j < k_maxNumberOfCells);
  return &m_floatCells[j];
}

ViewController * ValuesController::functionParameterController() {
  bool isDerivative = isDerivativeColumn(selectedColumn());
  if (isDerivative) {
    m_derivativeParameterController.setRecord(recordAtColumn(selectedColumn()));
    return &m_derivativeParameterController;
  }
  m_functionParameterController.setRecord(recordAtColumn(selectedColumn()));
  return &m_functionParameterController;
}

double ValuesController::evaluationOfAbscissaAtColumn(double abscissa, int columnIndex) {
  bool isDerivative = isDerivativeColumn(columnIndex);
  /* isDerivativeColumn uses expiring pointers, so "function" must be created
   * after the isDerivativeColumn call, else it will expire. */
  Shared::ExpiringPointer<CartesianFunction> function = functionStore()->modelForRecord(recordAtColumn(columnIndex));
  Poincare::Context * context = textFieldDelegateApp()->localContext();
  if (isDerivative) {
    return function->approximateDerivative(abscissa, context);
  }
  //TODO LEA RUBEN
  return function->evaluate2DAtParameter(abscissa, context).x1();
}

void ValuesController::updateNumberOfColumns() {
  int result = 1;
  for (int i = 0; i < functionStore()->numberOfActiveFunctions(); i++) {
    ExpiringPointer<CartesianFunction> f = functionStore()->modelForRecord(functionStore()->activeRecordAtIndex(i));
    if (f->isActive()) {
      result += 1 + f->displayDerivative();
    }
  }
  m_numberOfColumns = result;
}

}
