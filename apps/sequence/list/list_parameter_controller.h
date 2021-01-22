#ifndef SEQUENCE_LIST_PARAM_CONTROLLER_H
#define SEQUENCE_LIST_PARAM_CONTROLLER_H

#include <escher/even_odd_expression_cell.h>
#include <escher/message_table_cell_with_chevron_and_expression.h>
#include <escher/message_table_cell_with_chevron_and_message.h>
#include <escher/message_table_cell_with_editable_text.h>
#include <escher/message_table_cell_with_switch.h>
#include <escher/simple_list_view_data_source.h>
#include "../../shared/list_parameter_controller.h"
#include "../../shared/parameter_text_field_delegate.h"
#include "../../shared/sequence.h"
#include "../../shared/sequence_store.h"
#include "type_parameter_controller.h"

namespace Sequence {

class ListController;

class ListParameterController : public Shared::ListParameterController, public Escher::SelectableTableViewDelegate, public Shared::ParameterTextFieldDelegate {
public:
  ListParameterController(Escher::InputEventHandlerDelegate * inputEventHandlerDelegate, ListController * list);
  const char * title() override;
  bool handleEvent(Ion::Events::Event event) override;

  bool textFieldShouldFinishEditing(Escher::TextField * textField, Ion::Events::Event event) override;
  bool textFieldDidFinishEditing(Escher::TextField * textField, const char * text, Ion::Events::Event event) override;
  void tableViewDidChangeSelectionAndDidScroll(Escher::SelectableTableView * t, int previousSelectedCellX, int previousSelectedCellY, bool withinTemporarySelection) override;

  // SimpleListViewDataSource
  KDCoordinate nonMemoizedRowHeight(int j) override;
  Escher::HighlightCell * reusableCell(int index, int type) override;
  void willDisplayCellForIndex(Escher::HighlightCell * cell, int index) override;
private:
#if FUNCTION_COLOR_CHOICE
  constexpr static int k_totalNumberOfCell = 5;
#else
  constexpr static int k_totalNumberOfCell = 4;
#endif
  int totalNumberOfCells() const override;
  Shared::Sequence * sequence() { return static_cast<Shared::Sequence *>(function().pointer()); }
  bool hasInitialRankRow() const;
  Escher::MessageTableCellWithChevronAndExpression m_typeCell;
  Escher::MessageTableCellWithEditableText m_initialRankCell;
  TypeParameterController m_typeParameterController;
};

}

#endif
