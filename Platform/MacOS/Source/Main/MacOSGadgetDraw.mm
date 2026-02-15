#import "../../Include/MacOSGameWindowManager.h"
#include "GameClient/Display.h"
#include "GameClient/Gadget.h"
#include "GameClient/GadgetPushButton.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "PreRTS.h"

void MacOSGadgetPushButtonDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);

  Color color = 0xFF888888; // Default gray
  if (BitIsSet(instData->getState(), WIN_STATE_HILITED)) {
    color = 0xFFAAAAAA; // Lighter gray when hovered
  }
  if (BitIsSet(instData->getState(), WIN_STATE_SELECTED)) {
    color = 0xFF444444; // Darker gray when pressed
  }

  /*printf("MacOSGadgetPushButtonDraw: %d,%d %d,%d\n", origin.x, origin.y,
     size.x, size.y);*/
  TheWindowManager->winFillRect(color, 1.0, origin.x, origin.y,
                                origin.x + size.x, origin.y + size.y);
  TheWindowManager->winOpenRect(0xFFFFFFFF, 1.0, origin.x, origin.y,
                                origin.x + size.x, origin.y + size.y);
}

void MacOSGadgetPushButtonImageDraw(GameWindow *window,
                                    WinInstanceData *instData) {
  // For now, just use the same as standard pushbutton if no images
  MacOSGadgetPushButtonDraw(window, instData);
}

void MacOSGadgetStaticTextDraw(GameWindow *window, WinInstanceData *instData) {
  // Static text doesn't usually have a background
}

void MacOSGadgetCheckBoxDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);
  TheWindowManager->winOpenRect(0xFFFFFFFF, 1.0, origin.x, origin.y,
                                origin.x + 16, origin.y + 16);
}

void MacOSGadgetRadioButtonDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);
  TheWindowManager->winOpenRect(0xFFFFFFFF, 1.0, origin.x, origin.y,
                                origin.x + 16, origin.y + 16);
}

void MacOSGadgetTabControlDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);
  TheWindowManager->winFillRect(0xFF333333, 1.0, origin.x, origin.y,
                                origin.x + size.x, origin.y + size.y);
}

void MacOSGadgetListBoxDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);
  TheWindowManager->winFillRect(0xFF111111, 1.0, origin.x, origin.y,
                                origin.x + size.x, origin.y + size.y);
  TheWindowManager->winOpenRect(0xFF888888, 1.0, origin.x, origin.y,
                                origin.x + size.x, origin.y + size.y);
}

void MacOSGadgetComboBoxDraw(GameWindow *window, WinInstanceData *instData) {
  MacOSGadgetPushButtonDraw(window, instData);
}

void MacOSGadgetSliderDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);
  TheWindowManager->winDrawLine(0xFFFFFFFF, 2.0, origin.x,
                                origin.y + size.y / 2, origin.x + size.x,
                                origin.y + size.y / 2);
}

void MacOSGadgetProgressBarDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);
  TheWindowManager->winOpenRect(0xFFFFFFFF, 1.0, origin.x, origin.y,
                                origin.x + size.x, origin.y + size.y);
}

void MacOSGadgetTextEntryDraw(GameWindow *window, WinInstanceData *instData) {
  ICoord2D origin, size;
  window->winGetScreenPosition(&origin.x, &origin.y);
  window->winGetSize(&size.x, &size.y);
  TheWindowManager->winFillRect(0xFF000000, 1.0, origin.x, origin.y,
                                origin.x + size.x, origin.y + size.y);
  TheWindowManager->winOpenRect(0xFFFFFFFF, 1.0, origin.x, origin.y,
                                origin.x + size.x, origin.y + size.y);
}
