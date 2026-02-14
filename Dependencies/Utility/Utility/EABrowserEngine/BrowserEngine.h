#ifndef BROWSERENGINE_H
#define BROWSERENGINE_H

#include "win_compat.h"

struct IFEBrowserEngine2 {
  virtual void Initialize(long *device) {}
  virtual void Shutdown() {}
  virtual void D3DUpdate() {}
  virtual void D3DRender(int index) {}
  virtual void CreateBrowser(const char *name, const char *url, long hwnd,
                             int x, int y, int w, int h, long options,
                             void *dispatch) {}
  virtual void DestroyBrowser(const char *name) {}
  virtual int IsOpen(const char *name, long *isOpen = nullptr) { return 0; }
  virtual void Navigate(const char *name, const char *url) {}
  virtual void SetUpdateRate(const char *name, int ticks) {}
  virtual void put_BadPageURL(const char *url) {}
  virtual void put_LoadingPageURL(const char *url) {}
  virtual void put_MouseFileName(const char *name) {}
  virtual void put_MouseBusyFileName(const char *name) {}
};

struct FEBrowserEngine2 {};

#endif // BROWSERENGINE_H
