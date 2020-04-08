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

#ifndef LOTTIEKEYPATH_H
#define LOTTIEKEYPATH_H

#include "vglobal.h"

class LOTKeyPath{
public:
    LOTKeyPath(const rlottie_std::string &keyPath);
    bool matches(const rlottie_std::string &key, uint depth);
    uint nextDepth(const rlottie_std::string key, uint depth);
    bool fullyResolvesTo(const rlottie_std::string key, uint depth);

    bool propagate(const rlottie_std::string key, uint depth) {
        return skip(key) ? true : (depth < size()) || (mKeys[depth] == "**");
    }
    bool skip(const rlottie_std::string &key) const { return key == "__";}
private:
    bool isGlobstar(uint depth) const {return mKeys[depth] == "**";}
    bool isGlob(uint depth) const {return mKeys[depth] == "*";}
    bool endsWithGlobstar() const { return mKeys.back() == "**"; }
    size_t size() const {return mKeys.size() - 1;}
private:
    rlottie_std::vector<rlottie_std::string> mKeys;
};

#endif //LOTTIEKEYPATH_H
