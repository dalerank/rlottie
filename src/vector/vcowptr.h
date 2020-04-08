/* 
 * Copyright (c) 2018 Samsung Electronics Co., Ltd. All rights reserved.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef VCOWPTR_H
#define VCOWPTR_H

#include <cassert>
#include "vglobal.h"

#ifdef LOTTIE_DEFAULT_ALLOCATOR
#include <memory>

template <typename T>
class vcow_ptr {
  rlottie_std::shared_ptr<T> mModel;

public:
  vcow_ptr() { mModel = rlottie_std::make_shared<T>(); }
  ~vcow_ptr() {}

  template <class... Args>
  explicit vcow_ptr(Args&&... args) : mModel(rlottie_std::make_shared<T>(rlottie_std::forward<Args>(args)...)) {}

  vcow_ptr(const vcow_ptr& x) noexcept : mModel(x.mModel) {}
  vcow_ptr(vcow_ptr&& x) noexcept : mModel(x.mModel) {}

  vcow_ptr& operator=(const vcow_ptr& x) noexcept
  {
    *this = vcow_ptr(x);
    return *this;
  }

  vcow_ptr& operator=(vcow_ptr&& x) noexcept
  {
    auto tmp = rlottie_std::move(x);
    swap(*this, tmp);
    return *this;
  }

  const T& operator*() const noexcept { return read(); }
  const T* operator-> () const noexcept { return &read(); }

  size_t refCount() const noexcept { return mModel.use_count(); }
  bool unique() const noexcept { return mModel.unique(); }

  T& write()
  {
    if (!unique()) 
      *this = vcow_ptr(read());
    return *mModel.get();
  }

  const T& read() const noexcept { return *mModel.get(); }
  friend inline void swap(vcow_ptr& x, vcow_ptr& y) noexcept { rlottie_std::swap(x.mModel, y.mModel); }
};

#else

#include <atomic>

template <typename T>
class vcow_ptr {
    struct model {
        rlottie_std::atomic<rlottie_std::size_t> mRef{1};

        model() = default;

        template <class... Args>
        explicit model(Args&&... args) : mValue(rlottie_std::forward<Args>(args)...){}
        explicit model(const T& other) : mValue(other){}

        T mValue;
    };
    model* mModel;

public:
    using element_type = T;

    vcow_ptr()
    {
        static model default_s;
        mModel = &default_s;
        ++mModel->mRef;
    }

    ~vcow_ptr()
    {
        if (mModel && (--mModel->mRef == 0)) delete mModel;
    }

    template <class... Args>
    explicit vcow_ptr(Args&&... args) : mModel(new model(rlottie_std::forward<Args>(args)...))
    {
    }

    vcow_ptr(const vcow_ptr& x) noexcept : mModel(x.mModel)
    {
        assert(mModel);
        ++mModel->mRef;
    }
    vcow_ptr(vcow_ptr&& x) noexcept : mModel(x.mModel)
    {
        assert(mModel);
        x.mModel = nullptr;
    }

    auto operator=(const vcow_ptr& x) noexcept -> vcow_ptr&
    {
        *this = vcow_ptr(x);
        return *this;
    }

    auto operator=(vcow_ptr&& x) noexcept -> vcow_ptr&
    {
        auto tmp = rlottie_std::move(x);
        swap(*this, tmp);
        return *this;
    }

    auto operator*() const noexcept -> const element_type& { return read(); }

    auto operator-> () const noexcept -> const element_type* { return &read(); }

    rlottie_std::size_t refCount() const noexcept
    {
        assert(mModel);

        return mModel->mRef;
    }

    bool unique() const noexcept
    {
        assert(mModel);

        return mModel->mRef == 1;
    }

    auto write() -> element_type&
    {
        if (!unique()) *this = vcow_ptr(read());

        return mModel->mValue;
    }

    auto read() const noexcept -> const element_type&
    {
        assert(mModel);

        return mModel->mValue;
    }

    friend inline void swap(vcow_ptr& x, vcow_ptr& y) noexcept
    {
        rlottie_std::swap(x.mModel, y.mModel);
    }
};
#endif

#endif  // VCOWPTR_H
