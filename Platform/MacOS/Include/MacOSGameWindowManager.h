#pragma once

#include "GameClient/GameWindowManager.h"

class MacOSGameWindowManager : public GameWindowManager {
public:
  MacOSGameWindowManager(void);
  virtual ~MacOSGameWindowManager(void);

  virtual GameWindow *allocateNewWindow(void) override;

  virtual GameWinDrawFunc getPushButtonImageDrawFunc(void) override;
  virtual GameWinDrawFunc getPushButtonDrawFunc(void) override;
  virtual GameWinDrawFunc getCheckBoxImageDrawFunc(void) override;
  virtual GameWinDrawFunc getCheckBoxDrawFunc(void) override;
  virtual GameWinDrawFunc getRadioButtonImageDrawFunc(void) override;
  virtual GameWinDrawFunc getRadioButtonDrawFunc(void) override;
  virtual GameWinDrawFunc getTabControlImageDrawFunc(void) override;
  virtual GameWinDrawFunc getTabControlDrawFunc(void) override;
  virtual GameWinDrawFunc getListBoxImageDrawFunc(void) override;
  virtual GameWinDrawFunc getListBoxDrawFunc(void) override;
  virtual GameWinDrawFunc getComboBoxImageDrawFunc(void) override;
  virtual GameWinDrawFunc getComboBoxDrawFunc(void) override;
  virtual GameWinDrawFunc getHorizontalSliderImageDrawFunc(void) override;
  virtual GameWinDrawFunc getHorizontalSliderDrawFunc(void) override;
  virtual GameWinDrawFunc getVerticalSliderImageDrawFunc(void) override;
  virtual GameWinDrawFunc getVerticalSliderDrawFunc(void) override;
  virtual GameWinDrawFunc getProgressBarImageDrawFunc(void) override;
  virtual GameWinDrawFunc getProgressBarDrawFunc(void) override;
  virtual GameWinDrawFunc getStaticTextImageDrawFunc(void) override;
  virtual GameWinDrawFunc getStaticTextDrawFunc(void) override;
  virtual GameWinDrawFunc getTextEntryImageDrawFunc(void) override;
  virtual GameWinDrawFunc getTextEntryDrawFunc(void) override;
};
