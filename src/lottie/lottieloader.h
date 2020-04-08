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

#ifndef LOTTIELOADER_H
#define LOTTIELOADER_H

#include "rlottie.h"

class LOTModel;
class LottieLoader
{
public:
   static void configureModelCacheSize(size_t cacheSize);
   bool load(const rlottie_std::string &filePath, bool cachePolicy);
   bool loadFromData(rlottie_std::string &&jsonData, const rlottie_std::string &key,
                     const rlottie_std::string &resourcePath, bool cachePolicy);
   rlottie_std::shared_ptr<LOTModel> model();
private:  
   rlottie_std::shared_ptr<LOTModel>    mModel;
};

#endif // LOTTIELOADER_H


