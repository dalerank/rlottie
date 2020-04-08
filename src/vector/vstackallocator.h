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

#ifndef VSTACK_ALLOCATOR_H
#define VSTACK_ALLOCATOR_H

#include <cstddef>
#include <cassert>

template <rlottie_std::size_t N, rlottie_std::size_t alignment = alignof(rlottie_std::max_align_t)>
class arena
{
    alignas(alignment) char buf_[N];
    char* ptr_;

public:
    ~arena() {ptr_ = nullptr;}
    arena() noexcept : ptr_(buf_) {}
    arena(const arena&) = delete;
    arena& operator=(const arena&) = delete;

    template <rlottie_std::size_t ReqAlign> char* allocate(rlottie_std::size_t n);
    void deallocate(char* p, rlottie_std::size_t n) noexcept;

    static constexpr rlottie_std::size_t size() noexcept {return N;}
    rlottie_std::size_t used() const noexcept {return static_cast<rlottie_std::size_t>(ptr_ - buf_);}
    void reset() noexcept {ptr_ = buf_;}

private:
    static
    rlottie_std::size_t
    align_up(rlottie_std::size_t n) noexcept
        {return (n + (alignment-1)) & ~(alignment-1);}

    bool
    pointer_in_buffer(char* p) noexcept
        {return buf_ <= p && p <= buf_ + N;}
};

template <rlottie_std::size_t N, rlottie_std::size_t alignment>
template <rlottie_std::size_t ReqAlign>
char*
arena<N, alignment>::allocate(rlottie_std::size_t n)
{
    static_assert(ReqAlign <= alignment, "alignment is too small for this arena");
    assert(pointer_in_buffer(ptr_) && "stack_alloc has outlived arena");
    auto const aligned_n = align_up(n);
    if (static_cast<decltype(aligned_n)>(buf_ + N - ptr_) >= aligned_n)
    {
        char* r = ptr_;
        ptr_ += aligned_n;
        return r;
    }

    static_assert(alignment <= alignof(rlottie_std::max_align_t), "you've chosen an "
                  "alignment that is larger than alignof(rlottie_std::max_align_t), and "
                  "cannot be guaranteed by normal operator new");
    return static_cast<char*>(::operator new(n));
}

template <rlottie_std::size_t N, rlottie_std::size_t alignment>
void
arena<N, alignment>::deallocate(char* p, rlottie_std::size_t n) noexcept
{
    assert(pointer_in_buffer(ptr_) && "stack_alloc has outlived arena");
    if (pointer_in_buffer(p))
    {
        n = align_up(n);
        if (p + n == ptr_)
            ptr_ = p;
    }
    else
        ::operator delete(p);
}

template <class T, rlottie_std::size_t N, rlottie_std::size_t Align = alignof(rlottie_std::max_align_t)>
class stack_alloc
{
public:
    using value_type = T;
    static auto constexpr alignment = Align;
    static auto constexpr size = N;
    using arena_type = arena<size, alignment>;

private:
    arena_type& a_;

public:
    stack_alloc(const stack_alloc&) = default;
    stack_alloc& operator=(const stack_alloc&) = delete;

    stack_alloc(arena_type& a) noexcept : a_(a)
    {
        static_assert(size % alignment == 0,
                      "size N needs to be a multiple of alignment Align");
    }
    template <class U>
        stack_alloc(const stack_alloc<U, N, alignment>& a) noexcept
            : a_(a.a_) {}

    template <class _Up> struct rebind {using other = stack_alloc<_Up, N, alignment>;};

    T* allocate(rlottie_std::size_t n)
    {
        return reinterpret_cast<T*>(a_.template allocate<alignof(T)>(n*sizeof(T)));
    }
    void deallocate(T* p, rlottie_std::size_t n) noexcept
    {
        a_.deallocate(reinterpret_cast<char*>(p), n*sizeof(T));
    }

    template <class T1, rlottie_std::size_t N1, rlottie_std::size_t A1,
              class U, rlottie_std::size_t M, rlottie_std::size_t A2>
    friend
    bool
    operator==(const stack_alloc<T1, N1, A1>& x, const stack_alloc<U, M, A2>& y) noexcept;

    template <class U, rlottie_std::size_t M, rlottie_std::size_t A> friend class stack_alloc;
};

template <class T, rlottie_std::size_t N, rlottie_std::size_t A1, class U, rlottie_std::size_t M, rlottie_std::size_t A2>
inline
bool
operator==(const stack_alloc<T, N, A1>& x, const stack_alloc<U, M, A2>& y) noexcept
{
    return N == M && A1 == A2 && &x.a_ == &y.a_;
}

template <class T, rlottie_std::size_t N, rlottie_std::size_t A1, class U, rlottie_std::size_t M, rlottie_std::size_t A2>
inline
bool
operator!=(const stack_alloc<T, N, A1>& x, const stack_alloc<U, M, A2>& y) noexcept
{
    return !(x == y);
}

#endif  // VSTACK_ALLOCATOR_H
