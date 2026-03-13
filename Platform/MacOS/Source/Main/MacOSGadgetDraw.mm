#import "../../Include/MacOSGameWindowManager.h"
#include "GameClient/Display.h"
#include "GameClient/DisplayString.h"
#include "GameClient/Gadget.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowGlobal.h"
#include "GameClient/GameWindowManager.h"
#include "PreRTS.h"

#include "MacOSDebugLog.h"

// Helper to draw a beveled rectangle
static void DrawBeveledRect(Int x, Int y, Int w, Int h, Color bodyColor) {
  // Draw main body
  TheDisplay->drawFillRect(x, y, w, h, bodyColor);
  // Draw light top/left border
  TheDisplay->drawFillRect(x, y, w, 2, 0xFFAAAAAA);
  TheDisplay->drawFillRect(x, y, 2, h, 0xFFAAAAAA);
  // Draw dark bottom/right border
  TheDisplay->drawFillRect(x, y + h - 2, w, 2, 0xFF444444);
  TheDisplay->drawFillRect(x + w - 2, y, 2, h, 0xFF444444);
}

// Draw text label for a gadget
static void DrawGadgetText(GameWindow *window, WinInstanceData *instData) {
  DisplayString *ds = instData->getTextDisplayString();
  if (ds == nullptr || ds->getTextLength() == 0)
    return;

  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);

  // Set word wrap to button width
  ds->setWordWrapCentered(BitIsSet(instData->getStatus(), WIN_STATUS_WRAP_CENTERED));
  ds->setWordWrap(size.x);

  // Match font to window's font
  if (ds->getFont() != window->winGetFont())
    ds->setFont(window->winGetFont());

  // Get the right text color based on state
  Color textColor, dropColor;
  if (BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED) == FALSE) {
    textColor = window->winGetDisabledTextColor();
    dropColor = window->winGetDisabledTextBorderColor();
  } else if (BitIsSet(instData->getState(), WIN_STATE_HILITED)) {
    textColor = window->winGetHiliteTextColor();
    dropColor = window->winGetHiliteTextBorderColor();
  } else {
    textColor = window->winGetEnabledTextColor();
    dropColor = window->winGetEnabledTextBorderColor();
  }

  // If text color is still undefined or invisible, use fallback
  if (textColor == 0 || textColor == 0x00000000)
    textColor = 0xFFFFFFFF; // white fallback
  if (dropColor == 0)
    dropColor = 0xFF000000; // black outline fallback

  Int tw, th;
  ds->getSize(&tw, &th);
  // Center text
  Int tx = origin.x + (size.x - tw) / 2;
  Int ty = origin.y + (size.y - th) / 2;
  ds->draw(tx, ty, textColor, dropColor);
}

// Fallback draw for generic windows (like backdrops)
void MacOSGadgetDefaultDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);

  const Image *img = window->winGetEnabledImage(0);
  if (img) {
    TheWindowManager->winDrawImage(img, origin.x, origin.y, origin.x + size.x,
                                   origin.y + size.y, 0xFFFFFFFF);
  }
}

void MacOSGadgetPushButtonDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);

  DisplayString *ds = instData->getTextDisplayString();
  Int textLen = ds ? ds->getTextLength() : -1;
  DLOG_RFLOW(4, "MacOSPushButtonDraw pos=(%d,%d) size=(%dx%d) textLen=%d status=0x%x",
    origin.x, origin.y, size.x, size.y, textLen, (unsigned)window->winGetStatus());

  // Try to draw the appropriate image for the button state
  const Image *img = nullptr;
  if (BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED) == FALSE) {
    img = window->winGetDisabledImage(0);
  } else if (BitIsSet(instData->getState(), WIN_STATE_HILITED)) {
    img = window->winGetHiliteImage(0);
  }
  if (!img) {
    img = window->winGetEnabledImage(0);
  }

  if (img) {
    TheWindowManager->winDrawImage(img, origin.x, origin.y, origin.x + size.x,
                                   origin.y + size.y, 0xFFFFFFFF);
  } else {
    // Fallback: dark background if no texture
    DrawBeveledRect(origin.x, origin.y, size.x, size.y, 0xFF333333);
  }

  DrawGadgetText(window, instData);
}

void MacOSGadgetPushButtonImageDraw(GameWindow *window,
                                    WinInstanceData *instData) {
  DLOG_RFLOW(5, "MacOSPushButtonImageDraw -> forwarding to MacOSPushButtonDraw");
  MacOSGadgetPushButtonDraw(window, instData);
}

void MacOSGadgetStaticTextDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);

  const Image *img = window->winGetEnabledImage(0);
  if (img) {
    TheWindowManager->winDrawImage(img, origin.x, origin.y, origin.x + size.x,
                                   origin.y + size.y, 0xFFFFFFFF);
  }
}

void MacOSGadgetCheckBoxDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  DrawBeveledRect(origin.x, origin.y, 16, 16, 0xFF333333);
}

void MacOSGadgetRadioButtonDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  DrawBeveledRect(origin.x, origin.y, 16, 16, 0xFF333333);
}

void MacOSGadgetTabControlDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);
  DrawBeveledRect(origin.x, origin.y, size.x, size.y, 0xFF555555);
}

void MacOSGadgetListBoxDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);
  DrawBeveledRect(origin.x, origin.y, size.x, size.y, 0xFF111111);
}

void MacOSGadgetComboBoxDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);
  DrawBeveledRect(origin.x, origin.y, size.x, size.y, 0xFF777777);
}

void MacOSGadgetSliderDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);
  TheDisplay->drawFillRect(origin.x, origin.y + size.y / 2 - 1, size.x, 2,
                           0xFFFFFFFF);
}

void MacOSGadgetProgressBarDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);
  DrawBeveledRect(origin.x, origin.y, size.x, size.y, 0xFF222222);
  TheDisplay->drawFillRect(origin.x + 2, origin.y + 2, (size.x - 4) / 2,
                           size.y - 4, 0xFF00FF00); // 50% dummy
}

void MacOSGadgetTextEntryDraw(GameWindow *window, WinInstanceData *instData) {
  EntryData *e = (EntryData *)window->winGetUserData();
  ICoord2D origin, size;

  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);

  Color backBorder, backColor, textColor, textBorder;

  if (BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED) == FALSE) {
    textColor   = window->winGetDisabledTextColor();
    textBorder  = window->winGetDisabledTextBorderColor();
    backColor   = GadgetTextEntryGetDisabledColor(window);
    backBorder  = GadgetTextEntryGetDisabledBorderColor(window);
  } else if (BitIsSet(instData->getState(), WIN_STATE_HILITED)) {
    textColor   = window->winGetHiliteTextColor();
    textBorder  = window->winGetHiliteTextBorderColor();
    backColor   = GadgetTextEntryGetHiliteColor(window);
    backBorder  = GadgetTextEntryGetHiliteBorderColor(window);
  } else {
    textColor   = window->winGetEnabledTextColor();
    textBorder  = window->winGetEnabledTextBorderColor();
    backColor   = GadgetTextEntryGetEnabledColor(window);
    backBorder  = GadgetTextEntryGetEnabledBorderColor(window);
  }

  if (backBorder != WIN_COLOR_UNDEFINED) {
    TheWindowManager->winOpenRect(backBorder, WIN_DRAW_LINE_WIDTH,
                                  origin.x, origin.y,
                                  origin.x + size.x, origin.y + size.y);
  }

  if (backColor != WIN_COLOR_UNDEFINED) {
    TheWindowManager->winFillRect(backColor, WIN_DRAW_LINE_WIDTH,
                                  origin.x + 1, origin.y + 1,
                                  origin.x + size.x - 1, origin.y + size.y - 1);
  }

  if (!e) return;

  e->receivedUnichar = FALSE;

  if (textColor == WIN_COLOR_UNDEFINED) return;

  DisplayString *text = e->secretText ? e->sText : e->text;

  if (text->getFont() != window->winGetFont())
    text->setFont(window->winGetFont());

  Int fontHeight = TheWindowManager->winFontHeight(instData->getFont());
  Int startOffset = 5;
  Int width = size.x - (2 * startOffset);
  Int x = origin.x + startOffset;
  Int y;

  if (BitIsSet(window->winGetStatus(), WIN_STATUS_ONE_LINE))
    y = size.y / 2 - fontHeight / 2;
  else
    y = origin.y + startOffset;

  Int textWidth = text->getWidth();
  IRegion2D clipRegion;
  clipRegion.lo.x = x;
  clipRegion.hi.x = x + width;
  clipRegion.lo.y = y;
  clipRegion.hi.y = y + fontHeight;
  text->setClipRegion(&clipRegion);

  Int cursorPos;
  if (!e->drawTextFromStart) {
    x += 2;
    if (textWidth < width) {
      text->draw(x, y, textColor, textBorder);
      cursorPos = textWidth + x;
    } else {
      Int div = textWidth / (width / 2) - 1;
      text->draw(x - (div * (width / 2)), y, textColor, textBorder);
      cursorPos = textWidth - (div * (width / 2)) + x;
    }
  } else {
    x += 5;
    text->draw(x, y, textColor, textBorder);
    cursorPos = textWidth + x;
  }

  static Byte drawCnt = 0;
  GameWindow *parent = window->winGetParent();
  if (parent && !BitIsSet(parent->winGetStyle(), GWS_COMBO_BOX))
    parent = nullptr;

  if ((window == TheWindowManager->winGetFocus() ||
       (parent && parent == TheWindowManager->winGetFocus())) &&
      ((drawCnt++ >> 3) & 0x1)) {
    TheWindowManager->winFillRect(textColor, WIN_DRAW_LINE_WIDTH,
                                  cursorPos, origin.y + 3,
                                  cursorPos + 2, origin.y + size.y - 3);
  }

  window->winSetCursorPosition(cursorPos + 2 - origin.x, 0);
}

