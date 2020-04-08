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

#ifndef LOTTIEPROXYMODEL_H
#define LOTTIEPROXYMODEL_H

#include <cassert>
#include "lottiemodel.h"
#include "rlottie.h"

// Naive way to implement rlottie_std::variant
// refactor it when we move to c++17
// users should make sure proper combination
// of id and value are passed while creating the object.
class LOTVariant
{
public:
    using ValueFunc = rlottie_std::function<float(const rlottie::FrameInfo &)>;
    using ColorFunc = rlottie_std::function<rlottie::Color(const rlottie::FrameInfo &)>;
    using PointFunc = rlottie_std::function<rlottie::Point(const rlottie::FrameInfo &)>;
    using SizeFunc = rlottie_std::function<rlottie::Size(const rlottie::FrameInfo &)>;

    LOTVariant(rlottie::Property prop, const ValueFunc &v):mPropery(prop), mTag(Value)
    {
        construct(impl.valueFunc, v);
    }

    LOTVariant(rlottie::Property prop, ValueFunc &&v):mPropery(prop), mTag(Value)
    {
        moveConstruct(impl.valueFunc, rlottie_std::move(v));
    }

    LOTVariant(rlottie::Property prop, const ColorFunc &v):mPropery(prop), mTag(Color)
    {
        construct(impl.colorFunc, v);
    }

    LOTVariant(rlottie::Property prop, ColorFunc &&v):mPropery(prop), mTag(Color)
    {
        moveConstruct(impl.colorFunc, rlottie_std::move(v));
    }

    LOTVariant(rlottie::Property prop, const PointFunc &v):mPropery(prop), mTag(Point)
    {
        construct(impl.pointFunc, v);
    }

    LOTVariant(rlottie::Property prop, PointFunc &&v):mPropery(prop), mTag(Point)
    {
        moveConstruct(impl.pointFunc, rlottie_std::move(v));
    }

    LOTVariant(rlottie::Property prop, const SizeFunc &v):mPropery(prop), mTag(Size)
    {
        construct(impl.sizeFunc, v);
    }

    LOTVariant(rlottie::Property prop, SizeFunc &&v):mPropery(prop), mTag(Size)
    {
        moveConstruct(impl.sizeFunc, rlottie_std::move(v));
    }

    rlottie::Property property() const { return mPropery; }

    const ColorFunc& color() const
    {
        assert(mTag == Color);
        return impl.colorFunc;
    }

    const ValueFunc& value() const
    {
        assert(mTag == Value);
        return impl.valueFunc;
    }

    const PointFunc& point() const
    {
        assert(mTag == Point);
        return impl.pointFunc;
    }

    const SizeFunc& size() const
    {
        assert(mTag == Size);
        return impl.sizeFunc;
    }

    LOTVariant() = default;
    ~LOTVariant() noexcept {Destroy();}
    LOTVariant(const LOTVariant& other) { Copy(other);}
    LOTVariant(LOTVariant&& other) noexcept { Move(rlottie_std::move(other));}
    LOTVariant& operator=(LOTVariant&& other) { Destroy(); Move(rlottie_std::move(other)); return *this;}
    LOTVariant& operator=(const LOTVariant& other) { Destroy(); Copy(other); return *this;}
private:
    template <typename T>
    void construct(T& member, const T& val)
    {
        new (&member) T(val);
    }

    template <typename T>
    void moveConstruct(T& member, T&& val)
    {
        new (&member) T(rlottie_std::move(val));
    }

    void Move(LOTVariant&& other)
    {
        switch (other.mTag) {
        case Type::Value:
            moveConstruct(impl.valueFunc, rlottie_std::move(other.impl.valueFunc));
            break;
        case Type::Color:
            moveConstruct(impl.colorFunc, rlottie_std::move(other.impl.colorFunc));
            break;
        case Type::Point:
            moveConstruct(impl.pointFunc, rlottie_std::move(other.impl.pointFunc));
            break;
        case Type::Size:
            moveConstruct(impl.sizeFunc, rlottie_std::move(other.impl.sizeFunc));
            break;
        default:
            break;
        }
        mTag = other.mTag;
        mPropery = other.mPropery;
        other.mTag = MonoState;
    }

    void Copy(const LOTVariant& other)
    {
        switch (other.mTag) {
        case Type::Value:
            construct(impl.valueFunc, other.impl.valueFunc);
            break;
        case Type::Color:
            construct(impl.colorFunc, other.impl.colorFunc);
            break;
        case Type::Point:
            construct(impl.pointFunc, other.impl.pointFunc);
            break;
        case Type::Size:
            construct(impl.sizeFunc, other.impl.sizeFunc);
            break;
        default:
            break;
        }
        mTag = other.mTag;
        mPropery = other.mPropery;
    }

    void Destroy()
    {
        switch(mTag) {
        case MonoState: {
            break;
        }
        case Value: {
            impl.valueFunc.~ValueFunc();
            break;
        }
        case Color: {
            impl.colorFunc.~ColorFunc();
            break;
        }
        case Point: {
            impl.pointFunc.~PointFunc();
            break;
        }
        case Size: {
            impl.sizeFunc.~SizeFunc();
            break;
        }
        }
    }

    enum Type {MonoState, Value, Color, Point , Size};
    rlottie::Property mPropery;
    Type              mTag{MonoState};
    union details{
      ColorFunc   colorFunc;
      ValueFunc   valueFunc;
      PointFunc   pointFunc;
      SizeFunc    sizeFunc;
      details(){}
      ~details(){}
    }impl;
};

class LOTFilter
{
public:
    void addValue(LOTVariant &value)
    {
        uint index = static_cast<uint>(value.property());
        if (mBitset.test(index)) {
            rlottie_std::replace_if(mFilters.begin(),
                            mFilters.end(),
                            [&value](const LOTVariant &e) {return e.property() == value.property();},
                            value);
        } else {
            mBitset.set(index);
            mFilters.push_back(value);
        }
    }

    void removeValue(LOTVariant &value)
    {
        uint index = static_cast<uint>(value.property());
        if (mBitset.test(index)) {
            mBitset.reset(index);
            mFilters.erase(rlottie_std::remove_if(mFilters.begin(),
                                          mFilters.end(),
                                          [&value](const LOTVariant &e) {return e.property() == value.property();}),
                           mFilters.end());
        }
    }
    bool hasFilter(rlottie::Property prop) const
    {
        return mBitset.test(static_cast<uint>(prop));
    }
    LottieColor color(rlottie::Property prop, int frame) const
    {
        rlottie::FrameInfo info(frame);
        rlottie::Color col = data(prop).color()(info);
        return LottieColor(col.r(), col.g(), col.b());
    }
    VPointF point(rlottie::Property prop, int frame) const
    {
        rlottie::FrameInfo info(frame);
        rlottie::Point pt = data(prop).point()(info);
        return VPointF(pt.x(), pt.y());
    }
    VSize scale(rlottie::Property prop, int frame) const
    {
        rlottie::FrameInfo info(frame);
        rlottie::Size sz = data(prop).size()(info);
        return VSize(sz.w(), sz.h());
    }
    float opacity(rlottie::Property prop, int frame) const
    {
        rlottie::FrameInfo info(frame);
        float val = data(prop).value()(info);
        return val/100;
    }
    float value(rlottie::Property prop, int frame) const
    {
        rlottie::FrameInfo info(frame);
        return data(prop).value()(info);
    }
private:
    const LOTVariant& data(rlottie::Property prop) const
    {
        auto result = rlottie_std::find_if(mFilters.begin(),
                                   mFilters.end(),
                                   [prop](const LOTVariant &e){return e.property() == prop;});
        return *result;
    }
    rlottie_std::bitset<32>            mBitset{0};
    rlottie_std::vector<LOTVariant>    mFilters;
};

template <typename T>
class LOTProxyModel
{
public:
    LOTProxyModel(T *model): _modelData(model) {}
    LOTFilter& filter() {return mFilter;}
    const char* name() const {return _modelData->name();}
    LottieColor color(int frame) const
    {
        if (mFilter.hasFilter(rlottie::Property::StrokeColor)) {
            return mFilter.color(rlottie::Property::StrokeColor, frame);
        }
        return _modelData->color(frame);
    }
    float opacity(int frame) const
    {
        if (mFilter.hasFilter(rlottie::Property::StrokeOpacity)) {
            return mFilter.opacity(rlottie::Property::StrokeOpacity, frame);
        }
        return _modelData->opacity(frame);
    }
    float strokeWidth(int frame) const
    {
        if (mFilter.hasFilter(rlottie::Property::StrokeWidth)) {
            return mFilter.value(rlottie::Property::StrokeWidth, frame);
        }
        return _modelData->strokeWidth(frame);
    }
    float miterLimit() const {return _modelData->miterLimit();}
    CapStyle capStyle() const {return _modelData->capStyle();}
    JoinStyle joinStyle() const {return _modelData->joinStyle();}
    bool hasDashInfo() const { return _modelData->hasDashInfo();}
    void getDashInfo(int frameNo, rlottie_std::vector<float>& result) const {
        return _modelData->getDashInfo(frameNo, result);
    }

private:
    T                         *_modelData;
    LOTFilter                  mFilter;
};

template <>
class LOTProxyModel<LOTFillData>
{
public:
    LOTProxyModel(LOTFillData *model): _modelData(model) {}
    LOTFilter& filter() {return mFilter;}
    const char* name() const {return _modelData->name();}
    LottieColor color(int frame) const
    {
        if (mFilter.hasFilter(rlottie::Property::FillColor)) {
            return mFilter.color(rlottie::Property::FillColor, frame);
        }
        return _modelData->color(frame);
    }
    float opacity(int frame) const
    {
        if (mFilter.hasFilter(rlottie::Property::FillOpacity)) {
            return mFilter.opacity(rlottie::Property::FillOpacity, frame);
        }
        return _modelData->opacity(frame);
    }
    FillRule fillRule() const {return _modelData->fillRule();}
private:
    LOTFillData               *_modelData;
    LOTFilter                  mFilter;
};

template <>
class LOTProxyModel<LOTGroupData>
{
public:
    LOTProxyModel(LOTGroupData *model = nullptr): _modelData(model) {}
    bool hasModel() const { return _modelData ? true : false; }
    LOTFilter& filter() {return mFilter;}
    const char* name() const {return _modelData->name();}
    LOTTransformData* transform() const { return _modelData->mTransform; }
    VMatrix matrix(int frame) const
    {
        VMatrix mS, mR, mT;
        if (mFilter.hasFilter(rlottie::Property::TrScale)) {
            VSize s = mFilter.scale(rlottie::Property::TrScale, frame);
            mS.scale(s.width() / 100.0, s.height() / 100.0);
        }
        if (mFilter.hasFilter(rlottie::Property::TrRotation)) {
            mR.rotate(mFilter.value(rlottie::Property::TrRotation, frame));
        }
        if (mFilter.hasFilter(rlottie::Property::TrPosition)) {
            mT.translate(mFilter.point(rlottie::Property::TrPosition, frame));
        }

        return _modelData->mTransform->matrix(frame) * mS * mR * mT;
    }
private:
    LOTGroupData               *_modelData;
    LOTFilter                  mFilter;
};
#endif // LOTTIEITEM_H
