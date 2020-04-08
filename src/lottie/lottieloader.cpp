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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "lottieloader.h"
#include "lottieparser.h"

#include <cstring>
#include <stdio.h>

#ifdef LOTTIE_CACHE_SUPPORT

#include <mutex>

class LottieModelCache {
public:
    static LottieModelCache &instance()
    {
        static LottieModelCache CACHE;
        return CACHE;
    }
    rlottie_std::shared_ptr<LOTModel> find(const rlottie_std::string &key)
    {
        rlottie_std::lock_guard<rlottie_std::mutex> guard(mMutex);

        if (!mcacheSize) return nullptr;

        auto search = mHash.find(key);

        return (search != mHash.end()) ? search->second : nullptr;

    }
    void add(const rlottie_std::string &key, rlottie_std::shared_ptr<LOTModel> value)
    {
        rlottie_std::lock_guard<rlottie_std::mutex> guard(mMutex);

        if (!mcacheSize) return;

        //@TODO just remove the 1st element
        // not the best of LRU logic
        if (mcacheSize == mHash.size()) mHash.erase(mHash.cbegin());

        mHash[key] = rlottie_std::move(value);
    }

    void configureCacheSize(size_t cacheSize)
    {
        rlottie_std::lock_guard<rlottie_std::mutex> guard(mMutex);
        mcacheSize = cacheSize;

        if (!mcacheSize) mHash.clear();
    }

private:
    LottieModelCache() = default;

    rlottie_std::unordered_map<rlottie_std::string, rlottie_std::shared_ptr<LOTModel>>  mHash;
    rlottie_std::mutex                                                  mMutex;
    size_t                                                      mcacheSize{10};
};

#else

class LottieModelCache {
public:
    static LottieModelCache &instance()
    {
        static LottieModelCache CACHE;
        return CACHE;
    }
    rlottie_std::shared_ptr<LOTModel> find(const rlottie_std::string &) { return nullptr; }
    void add(const rlottie_std::string &, rlottie_std::shared_ptr<LOTModel>) {}
    void configureCacheSize(size_t) {}
};

#endif

void LottieLoader::configureModelCacheSize(size_t cacheSize)
{
    LottieModelCache::instance().configureCacheSize(cacheSize);
}

static rlottie_std::string dirname(const rlottie_std::string &path)
{
    const char *ptr = strrchr(path.c_str(), '/');
#ifdef _WIN32
    if (ptr) ptr = strrchr(ptr + 1, '\\');
#endif
    int         len = int(ptr + 1 - path.c_str());  // +1 to include '/'
    return rlottie_std::string(path, 0, len);
}

bool LottieLoader::load(const rlottie_std::string &path, bool cachePolicy)
{
    if (cachePolicy) {
        mModel = LottieModelCache::instance().find(path);
        if (mModel) return true;
    }

#ifdef LOTTIE_NOSTDSTREAM_SUPPORT
    FILE* f = fopen(path.c_str(), "rb");

    if (!f) {
        return false;
    }
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);

    rlottie_std::vector<char> where(size);

    rewind(f);
    fread(where.data(), sizeof(char), size, f);
    rlottie_std::string content(where.data(), where.size());
#else    
    rlottie_std::ifstream f;
    f.open(path);

    if (!f.is_open()) {
        vCritical << "failed to open file = " << path.c_str();
        return false;
    }

    rlottie_std::string content;

    rlottie_std::getline(f, content, '\0') ;
    f.close();
#endif

    if (content.empty()) return false;

    const char *str = content.c_str();
    LottieParser parser(const_cast<char *>(str),
                        dirname(path).c_str());
    mModel = parser.model();

    if (!mModel) return false;

    if (cachePolicy)
        LottieModelCache::instance().add(path, mModel);

    return true;
}

bool LottieLoader::loadFromData(rlottie_std::string &&jsonData, const rlottie_std::string &key,
                                const rlottie_std::string &resourcePath, bool cachePolicy)
{
    if (cachePolicy) {
        mModel = LottieModelCache::instance().find(key);
        if (mModel) return true;
    }

    LottieParser parser(const_cast<char *>(jsonData.c_str()),
                        resourcePath.c_str());
    mModel = parser.model();

    if (!mModel) return false;

    if (cachePolicy)
        LottieModelCache::instance().add(key, mModel);

    return true;
}

rlottie_std::shared_ptr<LOTModel> LottieLoader::model()
{
    return mModel;
}
