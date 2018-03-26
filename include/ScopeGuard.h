//
// Created by Yevgeni Kamenski on 11/5/17.
// From: https://rnestler.github.io/c-list-of-scopeguard.html
//

#pragma once

#include <algorithm>

struct ScopeGuardBase {
  ScopeGuardBase() : mActive(true) {}

  ScopeGuardBase(ScopeGuardBase &&rhs) noexcept
      : mActive(rhs.mActive) { rhs.dismiss(); }

  void dismiss() noexcept { mActive = false; }

 protected:
  virtual ~ScopeGuardBase() = default;
  bool mActive;
};

template<class Fun>
struct ScopeGuard : public ScopeGuardBase {
  ScopeGuard() = delete;
  ScopeGuard(const ScopeGuard &) = delete;

  explicit ScopeGuard(Fun f) noexcept:
      ScopeGuardBase(),
      mF(std::move(f)) {}

  ScopeGuard(ScopeGuard &&rhs) noexcept :
      ScopeGuardBase(std::move(rhs)),
      mF(std::move(rhs.mF)) {}

  ~ScopeGuard() noexcept override {
    if (mActive) {
      try { mF(); } catch (...) {}
    }
  }

  ScopeGuard &operator=(const ScopeGuard &) = delete;

 private:
  Fun mF;
};

template<class Fun>
ScopeGuard<Fun> make_guard(Fun f) {
  return ScopeGuard<Fun>(std::move(f));
}


