/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
*/

#pragma once

#include "GameClient/Mouse.h"

class CameraClass;
class RenderObjClass;
class HAnimClass;

class StdMouse : public Mouse {
public:
  StdMouse(void);
  virtual ~StdMouse(void);

  virtual void init(void) override;
  virtual void reset(void) override;
  virtual void update(void) override;
  virtual void initCursorResources(void) override;

  virtual void setCursor(MouseCursor cursor) override;
  virtual void setVisibility(Bool visible) override;
  virtual void draw(void) override;
  virtual void setRedrawMode(RedrawMode mode) override;

  virtual void loseFocus() override;
  virtual void regainFocus() override;

protected:
  virtual void capture(void) override;
  virtual void releaseCapture(void) override;

  virtual UnsignedByte getMouseEvent(MouseIO *result, Bool flush) override;

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

private:
  // W3D 3D model cursor support
  void initW3DAssets();
  void freeW3DAssets();

  CameraClass *m_w3dCamera;
  MouseCursor m_currentW3DCursor;
  RenderObjClass *m_cursorModels[NUM_MOUSE_CURSORS];
  HAnimClass *m_cursorAnims[NUM_MOUSE_CURSORS];
  bool m_w3dAssetsLoaded;

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
  MACOS_MOUSE_LBUTTON_DBLCLK,
  MACOS_MOUSE_RBUTTON_DOWN,
  MACOS_MOUSE_RBUTTON_UP,
  MACOS_MOUSE_RBUTTON_DBLCLK,
  MACOS_MOUSE_MBUTTON_DOWN,
  MACOS_MOUSE_MBUTTON_UP,
  MACOS_MOUSE_WHEEL,
};
