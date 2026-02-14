#pragma once
#ifndef _WIN32

#include "objbase.h"

// Minimal ATL stubs
template <class T> class CComPtr {
public:
  T *p;
  CComPtr() : p(nullptr) {}
  CComPtr(T *lp) : p(lp) {
    if (p)
      p->AddRef();
  }
  ~CComPtr() {
    if (p)
      p->Release();
  }
  T **operator&() { return &p; }
  T *operator->() { return p; }
  operator T *() const { return p; }
  T *operator=(T *lp) {
    if (p)
      p->Release();
    p = lp;
    if (p)
      p->AddRef();
    return p;
  }
};

class CComModule {
public:
  HRESULT Init(void *, HINSTANCE, const GUID * = nullptr) { return 0; }
  void Term() {}
};

template <class T> class CComObject : public T {
public:
  CComObject() : m_dwRef(1) {}
  virtual ~CComObject() {}

  virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
                                                   void **ppvObject) override {
    (void)riid;
    if (!ppvObject)
      return (HRESULT)0x80004003L;
    *ppvObject = this;
    return 0;
  }
  virtual ULONG STDMETHODCALLTYPE AddRef() override { return ++m_dwRef; }
  virtual ULONG STDMETHODCALLTYPE Release() override {
    if (--m_dwRef == 0) {
      delete this;
      return 0;
    }
    return m_dwRef;
  }

  static HRESULT CreateInstance(CComObject<T> **pp) {
    if (!pp)
      return (HRESULT)0x80004003L; // E_POINTER
    *pp = new CComObject<T>();
    if (!*pp)
      return (HRESULT)0x8007000EL; // E_OUTOFMEMORY
    return 0;                      // S_OK
  }

private:
  ULONG m_dwRef;
};

template <class T> class CComQIPtr : public CComPtr<T> {
public:
  CComQIPtr() : CComPtr<T>() {}
  CComQIPtr(T *lp) : CComPtr<T>(lp) {}
  CComQIPtr(IUnknown *lp) : CComPtr<T>() {
    if (lp)
      lp->QueryInterface(IID_IUnknown, (void **)&this->p); // Stub
  }
};

#ifndef I_ID
#define I_ID(type) IID_##type
#endif

template <const IID &iid> class CComQIIDPtr : public CComPtr<IUnknown> {
public:
  CComQIIDPtr() : CComPtr<IUnknown>() {}
  CComQIIDPtr(IUnknown *lp) : CComPtr<IUnknown>() {
    if (lp)
      lp->QueryInterface(iid, (void **)&p);
  }
};

#endif
