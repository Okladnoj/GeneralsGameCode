/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
*/

#pragma once

#include "GameClient/Mouse.h"

class StdMouse : public Mouse {
public:
  StdMouse(void);
  virtual ~StdMouse(void);

  virtual void init(void);
  virtual void reset(void);
  virtual void update(void);
  virtual void initCursorResources(void);

  virtual void setCursor(MouseCursor cursor);
  virtual void setVisibility(Bool visible);

  virtual void loseFocus();
  virtual void regainFocus();

protected:
  virtual void capture(void);
  virtual void releaseCapture(void);

  virtual UnsignedByte getMouseEvent(MouseIO *result, Bool flush);

  struct MacOSMouseEvent {
    int type;
    int x, y;
    int button;
    int wheelDelta;
    unsigned int time;
  };

  enum { MAX_EVENTS = 256 };
  MacOSMouseEvent m_eventBuffer[MAX_EVENTS];
  unsigned int m_nextFreeIndex;
  unsigned int m_nextGetIndex;

public:
  // This will be called from the macOS event loop bridge
  void addEvent(int type, int x, int y, int button, int wheelDelta,
                unsigned int time);
};

// Types for bridge
enum MacOSMouseEventType {
  MACOS_MOUSE_MOVE,
  MACOS_MOUSE_LBUTTON_DOWN,
  MACOS_MOUSE_LBUTTON_UP,
  MACOS_MOUSE_RBUTTON_DOWN,
  MACOS_MOUSE_RBUTTON_UP,
  MACOS_MOUSE_MBUTTON_DOWN,
  MACOS_MOUSE_MBUTTON_UP,
  MACOS_MOUSE_WHEEL,
};
