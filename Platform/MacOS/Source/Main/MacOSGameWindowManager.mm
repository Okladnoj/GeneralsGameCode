#import "../../Include/MacOSGameWindowManager.h"
#include "Common/Debug.h"
#include "Common/GameMemory.h"
#import "GameClient/Gadget.h"
#import "GameClient/GameWindow.h"
#include "PreRTS.h"

// Simple MacOS GameWindow implementation
class MacOSGameWindow : public GameWindow {
  MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(MacOSGameWindow, "MacOSGameWindow")
public:
  virtual void winDrawBorder(void) override {
    // Basic border drawing could go here if needed
  }
  virtual void *winGetUserData(void) override { return nullptr; }
};

MacOSGameWindow::~MacOSGameWindow() {}

MacOSGameWindowManager::MacOSGameWindowManager(void) {}
MacOSGameWindowManager::~MacOSGameWindowManager(void) {}

GameWindow *MacOSGameWindowManager::allocateNewWindow(void) {
  return newInstance(MacOSGameWindow);
}

// For now, return default empty draw functions or stubs
GameWinDrawFunc MacOSGameWindowManager::getPushButtonImageDrawFunc(void) {
  return GameWinDefaultDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getPushButtonDrawFunc(void) {
  return GameWinDefaultDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getCheckBoxImageDrawFunc(void) {
  return GameWinDefaultDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getCheckBoxDrawFunc(void) {
  return GameWinDefaultDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getRadioButtonImageDrawFunc(void) {
  return GameWinDefaultDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getRadioButtonDrawFunc(void) {
  return GameWinDefaultDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getTabControlImageDrawFunc(void) {
  return GameWinDefaultDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getTabControlDrawFunc(void) {
  return GameWinDefaultDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getListBoxImageDrawFunc(void) {
  return GameWinDefaultDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getListBoxDrawFunc(void) {
  return GameWinDefaultDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getComboBoxImageDrawFunc(void) {
  return GameWinDefaultDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getComboBoxDrawFunc(void) {
  return GameWinDefaultDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getHorizontalSliderImageDrawFunc(void) {
  return GameWinDefaultDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getHorizontalSliderDrawFunc(void) {
  return GameWinDefaultDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getVerticalSliderImageDrawFunc(void) {
  return GameWinDefaultDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getVerticalSliderDrawFunc(void) {
  return GameWinDefaultDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getProgressBarImageDrawFunc(void) {
  return GameWinDefaultDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getProgressBarDrawFunc(void) {
  return GameWinDefaultDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getStaticTextImageDrawFunc(void) {
  return GameWinDefaultDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getStaticTextDrawFunc(void) {
  return GameWinDefaultDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getTextEntryImageDrawFunc(void) {
  return GameWinDefaultDraw;
}
GameWinDrawFunc MacOSGameWindowManager::getTextEntryDrawFunc(void) {
  return GameWinDefaultDraw;
}
