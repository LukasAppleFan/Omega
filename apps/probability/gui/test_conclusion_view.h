#ifndef APPS_PROBABILITY_GUI_TEST_CONCLUSION_VIEW_H
#define APPS_PROBABILITY_GUI_TEST_CONCLUSION_VIEW_H

#include <escher/expression_view.h>
#include <escher/palette.h>

#include "probability/gui/horizontal_or_vertical_layout.h"
#include "probability/gui/highlight_image_cell.h"

namespace Probability {

/* This view displays a message "test has been rejected / can't be rejected"
 * and a matching icon. */
class TestConclusionView : public HorizontalLayout {
public:
  enum class Type { Success, Failure };
  TestConclusionView();
  void setType(Type t);
  int numberOfSubviews() const override { return 3; }
  Escher::View * subviewAtIndex(int i) override;
  void layoutSubviews(bool force) override;
  KDSize minimalSizeForOptimalDisplay() const override;

private:
  constexpr static int k_marginBetween = 0;
  constexpr static int k_marginLeft = 20;
  constexpr static int k_iconSize = 14;

  HighlightImageCell m_icon;
  Escher::ExpressionView m_textView1;
  Escher::MessageTextView m_textView2;
};

}  // namespace Probability

#endif /* APPS_PROBABILITY_GUI_TEST_CONCLUSION_VIEW_H */
