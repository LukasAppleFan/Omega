#include <kernel/drivers/config/board.h>
#include <kernel/drivers/authentication.h>
#include <kernel/drivers/backlight.h>
#include <kernel/drivers/cortex_control.h>
#include <kernel/drivers/display.h>
#include <kernel/drivers/timing.h>
#include <ion/display.h>
#include <kandinsky/font.h>
#include <string.h>

extern "C" {
  extern char _kernel_start;
}

constexpr static int sNumberOfMessages = 7;
constexpr static const char * sMessages[sNumberOfMessages] = {
  "NON-AUTHENTICATED SOFTWARE",
  "Caution: you're using an",
  "unofficial software version.",
  "NumWorks can't be held responsible",
  "for any resulting damage.",
  "Some features (blue and green LED)",
  "are unavailable."
};

void drawString(const char * message, KDCoordinate & yOffset, const KDFont * font) {
  KDSize glyphSize = font->glyphSize();
  KDFont::RenderPalette palette = font->renderPalette(KDColorBlack, KDColorWhite);
  KDFont::GlyphBuffer glyphBuffer;
  size_t len = strlen(message);
  KDPoint position((Ion::Display::Width - len*glyphSize.width())/2, yOffset);

  while (*message) {
    font->setGlyphGrayscalesForCharacter(*message++, &glyphBuffer);
    font->colorizeGlyphBuffer(&palette, &glyphBuffer);
    // Push the character on the screen
    Ion::Device::Display::pushRect(KDRect(position, glyphSize), glyphBuffer.colorBuffer());
    position = position.translatedBy(KDPoint(glyphSize.width(), 0));
  }
  yOffset += glyphSize.height();
}

void displayWarningMessage() {
  KDRect screen = KDRect(0,0,Ion::Display::Width,Ion::Display::Height);
  Ion::Device::Display::pushRectUniform(screen, KDColorWhite);
  // TODO EMILIE: Would it be more optimized to directly use an image of the warning?
  const char * title = sMessages[0];
  KDCoordinate currentHeight = 60;
  drawString(title, currentHeight, KDFont::LargeFont);
  currentHeight += KDFont::LargeFont->glyphSize().height();
  for (int j = 1; j < sNumberOfMessages; j++) {
    const char * message = sMessages[j];
    drawString(message, currentHeight, KDFont::SmallFont);
  }
  Ion::Device::Timing::msleep(5000);
}

typedef void (*EntryPoint)();

void kernel_main() {
  // Display warning for unauthenticated software
  if (!Ion::Device::Authentication::trustedUserland()) {
    Ion::Device::Backlight::init();
    displayWarningMessage();
    Ion::Device::Backlight::shutdown();
  }
  // Unprivileged mode
  switch_to_unpriviledged();

  // Jump to userland
  EntryPoint * userlandFirstAddress = reinterpret_cast<EntryPoint *>(_kernel_start + Ion::Device::Board::Config::UserlandOffsetFromKernel);
  (*userlandFirstAddress)();
}
