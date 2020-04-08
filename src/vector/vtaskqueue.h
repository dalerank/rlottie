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

#ifndef VTASKQUEUE_H
#define VTASKQUEUE_H

#include "vglobal.h"

template <typename Task>
class TaskQueue {
    using lock_t = rlottie_std::unique_lock<rlottie_std::mutex>;
    rlottie_std::deque<Task>      _q;
    bool                    _done{false};
    rlottie_std::mutex              _mutex;
    rlottie_std::condition_variable _ready;

public:
    bool try_pop(Task &task)
    {
        lock_t lock{_mutex, rlottie_std::try_to_lock};
        if (!lock || _q.empty()) return false;
        task = rlottie_std::move(_q.front());
        _q.pop_front();
        return true;
    }

    bool try_push(Task &&task)
    {
        {
            lock_t lock{_mutex, rlottie_std::try_to_lock};
            if (!lock) return false;
            _q.push_back(rlottie_std::move(task));
        }
        _ready.notify_one();
        return true;
    }

    void done()
    {
        {
            lock_t lock{_mutex};
            _done = true;
        }
        _ready.notify_all();
    }

    bool pop(Task &task)
    {
        lock_t lock{_mutex};
        while (_q.empty() && !_done) _ready.wait(lock);
        if (_q.empty()) return false;
        task = rlottie_std::move(_q.front());
        _q.pop_front();
        return true;
    }

    void push(Task &&task)
    {
        {
            lock_t lock{_mutex};
            _q.push_back(rlottie_std::move(task));
        }
        _ready.notify_one();
    }

};

#endif  // VTASKQUEUE_H
