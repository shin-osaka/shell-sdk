/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos.com

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated engine source code (the "Software"), a limited,
 worldwide, royalty-free, non-assignable, revocable and non-exclusive license
 to use Cocos Creator solely to develop games on your target platforms. You shall
 not use Cocos Creator software for developing other software or tools that's
 used for developing games. You are not granted to publish, distribute,
 sublicense, and/or sell copies of Cocos Creator.

 The software or tools in this License Agreement are licensed, not sold.
 Xiamen Yaji Software Co., Ltd. reserves all rights not expressly granted to you.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "jsb_conversions.hpp"
#include <sstream>
#include <regex>

bool seval_to_int32(const se::Value &v, int32_t *ret)
{
    assert(ret != nullptr);
    if (v.isNumber())
    {
        *ret = v.toInt32();
        return true;
    }
    else if (v.isBoolean())
    {
        *ret = v.toBoolean() ? 1 : 0;
        return true;
    }
    *ret = 0;
    return false;
}

bool seval_to_uint32(const se::Value &v, uint32_t *ret)
{
    assert(ret != nullptr);
    if (v.isNumber())
    {
        *ret = v.toUint32();
        return true;
    }
    else if (v.isBoolean())
    {
        *ret = v.toBoolean() ? 1 : 0;
        return true;
    }
    *ret = 0;
    return false;
}

bool seval_to_int8(const se::Value &v, int8_t *ret)
{
    assert(ret != nullptr);
    if (v.isNumber())
    {
        *ret = v.toInt8();
        return true;
    }
    else if (v.isBoolean())
    {
        *ret = v.toBoolean() ? 1 : 0;
        return true;
    }
    *ret = 0;
    return false;
}

bool seval_to_uint8(const se::Value &v, uint8_t *ret)
{
    assert(ret != nullptr);
    if (v.isNumber())
    {
        *ret = v.toUint8();
        return true;
    }
    else if (v.isBoolean())
    {
        *ret = v.toBoolean() ? 1 : 0;
        return true;
    }
    *ret = 0;
    return false;
}

bool seval_to_int16(const se::Value &v, int16_t *ret)
{
    assert(ret != nullptr);
    if (v.isNumber())
    {
        *ret = v.toInt16();
        return true;
    }
    else if (v.isBoolean())
    {
        *ret = v.toBoolean() ? 1 : 0;
        return true;
    }
    *ret = 0;
    return false;
}

bool seval_to_uint16(const se::Value &v, uint16_t *ret)
{
    assert(ret != nullptr);
    if (v.isNumber())
    {
        *ret = v.toUint16();
        return true;
    }
    else if (v.isBoolean())
    {
        *ret = v.toBoolean() ? 1 : 0;
        return true;
    }
    *ret = 0;
    return false;
}

bool seval_to_boolean(const se::Value &v, bool *ret)
{
    assert(ret != nullptr);
    if (v.isBoolean())
    {
        *ret = v.toBoolean();
    }
    else if (v.isNumber())
    {
        *ret = v.toInt32() != 0 ? true : false;
    }
    else if (v.isNullOrUndefined())
    {
        *ret = false;
    }
    else if (v.isObject())
    {
        *ret = true;
    }
    else if (v.isString())
    {
        *ret = v.toString().empty() ? false : true;
    }
    else
    {
        *ret = false;
        assert(false);
    }

    return true;
}

bool seval_to_float(const se::Value &v, float *ret)
{
    assert(ret != nullptr);
    if (v.isNumber())
    {
        *ret = v.toFloat();
        if (!std::isnan(*ret))
            return true;
    }
    *ret = 0.0f;
    return false;
}

bool seval_to_double(const se::Value &v, double *ret)
{
    if (v.isNumber())
    {
        *ret = v.toNumber();
        if (!std::isnan(*ret))
            return true;
    }
    *ret = 0.0;
    return false;
}

bool seval_to_long(const se::Value &v, long *ret)
{
    assert(ret != nullptr);
    if (v.isNumber())
    {
        *ret = v.toLong();
        return true;
    }
    *ret = 0L;
    return false;
}

bool seval_to_ulong(const se::Value &v, unsigned long *ret)
{
    assert(ret != nullptr);
    if (v.isNumber())
    {
        *ret = v.toUlong();
        return true;
    }
    *ret = 0UL;
    return false;
}

bool seval_to_longlong(const se::Value &v, long long *ret)
{
    assert(ret != nullptr);
    if (v.isNumber())
    {
        *ret = (long long)v.toLong();
        return true;
    }
    *ret = 0LL;
    return false;
}

bool seval_to_ssize(const se::Value &v, ssize_t *ret)
{
    assert(ret != nullptr);
    if (v.isNumber())
    {
        *ret = (ssize_t)v.toLong();
        return true;
    }
    *ret = 0;
    return false;
}

bool seval_to_size(const se::Value &v, size_t *ret)
{
    assert(ret != nullptr);
    if (v.isNumber())
    {
        *ret = (size_t)v.toLong();
        return true;
    }
    *ret = 0;
    return false;
}

bool seval_to_std_string(const se::Value &v, std::string *ret)
{
    assert(ret != nullptr);
    *ret = v.toStringForce();
    return true;
}

bool seval_to_Vec2(const se::Value &v, cocos2d::Vec2 *pt)
{
    assert(pt != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to Vec2 failed!");
    se::Object *obj = v.toObject();
    se::Value x;
    se::Value y;
    bool ok = obj->getProperty("x", &x);
    SE_PRECONDITION3(ok && x.isNumber(), false, *pt = cocos2d::Vec2::ZERO);
    ok = obj->getProperty("y", &y);
    SE_PRECONDITION3(ok && y.isNumber(), false, *pt = cocos2d::Vec2::ZERO);
    pt->x = x.toFloat();
    pt->y = y.toFloat();
    return true;
}

bool seval_to_Vec3(const se::Value &v, cocos2d::Vec3 *pt)
{
    assert(pt != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to Vec3 failed!");
    se::Object *obj = v.toObject();
    se::Value x;
    se::Value y;
    se::Value z;
    bool ok = obj->getProperty("x", &x);
    SE_PRECONDITION3(ok && x.isNumber(), false, *pt = cocos2d::Vec3::ZERO);
    ok = obj->getProperty("y", &y);
    SE_PRECONDITION3(ok && y.isNumber(), false, *pt = cocos2d::Vec3::ZERO);
    ok = obj->getProperty("z", &z);
    SE_PRECONDITION3(ok && z.isNumber(), false, *pt = cocos2d::Vec3::ZERO);
    pt->x = x.toFloat();
    pt->y = y.toFloat();
    pt->z = z.toFloat();
    return true;
}

bool seval_to_Vec4(const se::Value &v, cocos2d::Vec4 *pt)
{
    assert(pt != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to Vec4 failed!");
    pt->x = pt->y = pt->z = pt->w = 0.0f;
    se::Object *obj = v.toObject();
    se::Value x;
    se::Value y;
    se::Value z;
    se::Value w;
    bool ok = obj->getProperty("x", &x);
    SE_PRECONDITION3(ok && x.isNumber(), false, *pt = cocos2d::Vec4::ZERO);
    ok = obj->getProperty("y", &y);
    SE_PRECONDITION3(ok && y.isNumber(), false, *pt = cocos2d::Vec4::ZERO);
    ok = obj->getProperty("z", &z);
    SE_PRECONDITION3(ok && z.isNumber(), false, *pt = cocos2d::Vec4::ZERO);
    ok = obj->getProperty("w", &w);
    SE_PRECONDITION3(ok && w.isNumber(), false, *pt = cocos2d::Vec4::ZERO);
    pt->x = x.toFloat();
    pt->y = y.toFloat();
    pt->z = z.toFloat();
    pt->w = w.toFloat();
    return true;
}

bool seval_to_mat(const se::Value &v, int length, float *out)
{
    assert(out != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to Matrix failed!");
    se::Object *obj = v.toObject();

    se::Value tmp;
    char propName[3] = {0};
    for (int i = 0; i < length; ++i)
    {
        snprintf(propName, 3, "m%2d", i);
        obj->getProperty(propName, &tmp);
        *(out + i) = tmp.toFloat();
    }

    return true;
}

bool seval_to_Mat4(const se::Value &v, cocos2d::Mat4 *mat)
{
    assert(mat != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to Matrix4 failed!");
    se::Object *obj = v.toObject();

    if (obj->isTypedArray())
    {
        SE_PRECONDITION2(obj->isTypedArray(), false, "Convert parameter to Matrix4 failed!");

        size_t length = 0;
        uint8_t *ptr = nullptr;
        obj->getTypedArrayData(&ptr, &length);

        memcpy(mat->m, ptr, length);
    }
    else
    {
        bool ok = false;
        se::Value tmp;
        std::string prefix = "m";
        for (uint32_t i = 0; i < 16; ++i)
        {
            std::string name;
            if (i < 10)
            {
                name = prefix + "0" + std::to_string(i);
            }
            else
            {
                name = prefix + std::to_string(i);
            }
            ok = obj->getProperty(name.c_str(), &tmp);
            SE_PRECONDITION3(ok, false, *mat = cocos2d::Mat4::IDENTITY);

            if (tmp.isNumber())
            {
                mat->m[i] = tmp.toFloat();
            }
            else
            {
                SE_REPORT_ERROR("%u, not supported type in matrix", i);
                *mat = cocos2d::Mat4::IDENTITY;
                return false;
            }

            tmp.setUndefined();
        }
    }

    return true;
}

bool seval_to_Size(const se::Value &v, cocos2d::Size *size)
{
    assert(size != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to Size failed!");
    se::Object *obj = v.toObject();
    se::Value width;
    se::Value height;

    bool ok = obj->getProperty("width", &width);
    SE_PRECONDITION3(ok && width.isNumber(), false, *size = cocos2d::Size::ZERO);
    ok = obj->getProperty("height", &height);
    SE_PRECONDITION3(ok && height.isNumber(), false, *size = cocos2d::Size::ZERO);
    size->width = width.toFloat();
    size->height = height.toFloat();
    return true;
}

bool seval_to_Color3B(const se::Value &v, cocos2d::Color3B *color)
{
    assert(color != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to Color3B failed!");
    se::Object *obj = v.toObject();
    se::Value r;
    se::Value g;
    se::Value b;
    bool ok = obj->getProperty("r", &r);
    SE_PRECONDITION3(ok && r.isNumber(), false, *color = cocos2d::Color3B::BLACK);
    ok = obj->getProperty("g", &g);
    SE_PRECONDITION3(ok && g.isNumber(), false, *color = cocos2d::Color3B::BLACK);
    ok = obj->getProperty("b", &b);
    SE_PRECONDITION3(ok && b.isNumber(), false, *color = cocos2d::Color3B::BLACK);
    color->r = (GLubyte)r.toUint16();
    color->g = (GLubyte)g.toUint16();
    color->b = (GLubyte)b.toUint16();
    return true;
}

bool seval_to_Color4B(const se::Value &v, cocos2d::Color4B *color)
{
    assert(color != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to Color4B failed!");
    se::Object *obj = v.toObject();
    se::Value r;
    se::Value g;
    se::Value b;
    se::Value a;
    bool ok = obj->getProperty("r", &r);
    SE_PRECONDITION3(ok && r.isNumber(), false, *color = cocos2d::Color4B::BLACK);
    ok = obj->getProperty("g", &g);
    SE_PRECONDITION3(ok && g.isNumber(), false, *color = cocos2d::Color4B::BLACK);
    ok = obj->getProperty("b", &b);
    SE_PRECONDITION3(ok && b.isNumber(), false, *color = cocos2d::Color4B::BLACK);
    ok = obj->getProperty("a", &a);
    SE_PRECONDITION3(ok && b.isNumber(), false, *color = cocos2d::Color4B::BLACK);
    color->r = (GLubyte)r.toUint16();
    color->g = (GLubyte)g.toUint16();
    color->b = (GLubyte)b.toUint16();
    color->a = (GLubyte)a.toUint16();
    return true;
}

bool seval_to_Color4F(const se::Value &v, cocos2d::Color4F *color)
{
    assert(color != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to Color4F failed!");
    se::Object *obj = v.toObject();
    se::Value r;
    se::Value g;
    se::Value b;
    se::Value a;
    bool ok = obj->getProperty("r", &r);
    SE_PRECONDITION3(ok && r.isNumber(), false, *color = cocos2d::Color4F::BLACK);
    ok = obj->getProperty("g", &g);
    SE_PRECONDITION3(ok && g.isNumber(), false, *color = cocos2d::Color4F::BLACK);
    ok = obj->getProperty("b", &b);
    SE_PRECONDITION3(ok && b.isNumber(), false, *color = cocos2d::Color4F::BLACK);
    ok = obj->getProperty("a", &a);
    SE_PRECONDITION3(ok && b.isNumber(), false, *color = cocos2d::Color4F::BLACK);
    color->r = r.toFloat();
    color->g = g.toFloat();
    color->b = b.toFloat();
    color->a = a.toFloat();
    return true;
}

bool seval_to_Color3F(const se::Value &v, cocos2d::Color3F *color)
{
    assert(color != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to Color3F failed!");
    se::Object *obj = v.toObject();
    se::Value r;
    se::Value g;
    se::Value b;
    bool ok = obj->getProperty("r", &r);
    SE_PRECONDITION3(ok && r.isNumber(), false, *color = cocos2d::Color3F::BLACK);
    ok = obj->getProperty("g", &g);
    SE_PRECONDITION3(ok && g.isNumber(), false, *color = cocos2d::Color3F::BLACK);
    ok = obj->getProperty("b", &b);
    SE_PRECONDITION3(ok && b.isNumber(), false, *color = cocos2d::Color3F::BLACK);
    color->r = r.toFloat();
    color->g = g.toFloat();
    color->b = b.toFloat();
    return true;
}

bool seval_to_ccvalue(const se::Value &v, cocos2d::Value *ret)
{
    assert(ret != nullptr);
    bool ok = true;
    if (v.isObject())
    {
        se::Object *jsobj = v.toObject();
        if (!jsobj->isArray())
        {
            cocos2d::ValueMap dictVal;
            ok = seval_to_ccvaluemap(v, &dictVal);
            SE_PRECONDITION3(ok, false, *ret = cocos2d::Value::Null);
            *ret = cocos2d::Value(dictVal);
        }
        else
        {
            cocos2d::ValueVector arrVal;
            ok = seval_to_ccvaluevector(v, &arrVal);
            SE_PRECONDITION3(ok, false, *ret = cocos2d::Value::Null);
            *ret = cocos2d::Value(arrVal);
        }
    }
    else if (v.isString())
    {
        *ret = v.toString();
    }
    else if (v.isNumber())
    {
        *ret = v.toNumber();
    }
    else if (v.isBoolean())
    {
        *ret = v.toBoolean();
    }
    else if (v.isNullOrUndefined())
    {
        *ret = cocos2d::Value::Null;
    }
    else
    {
        SE_PRECONDITION2(false, false, "type not supported!");
    }

    return ok;
}

bool seval_to_ccvaluemap(const se::Value &v, cocos2d::ValueMap *ret)
{
    assert(ret != nullptr);

    if (v.isNullOrUndefined())
    {
        ret->clear();
        return true;
    }

    SE_PRECONDITION3(v.isObject(), false, ret->clear());
    SE_PRECONDITION3(!v.isNullOrUndefined(), false, ret->clear());

    se::Object *obj = v.toObject();

    cocos2d::ValueMap &dict = *ret;

    std::vector<std::string> allKeys;
    SE_PRECONDITION3(obj->getAllKeys(&allKeys), false, ret->clear());

    bool ok = false;
    se::Value value;
    cocos2d::Value ccvalue;
    for (const auto &key : allKeys)
    {
        SE_PRECONDITION3(obj->getProperty(key.c_str(), &value), false, ret->clear());
        ok = seval_to_ccvalue(value, &ccvalue);
        SE_PRECONDITION3(ok, false, ret->clear());
        dict.emplace(key, ccvalue);
    }

    return true;
}

static bool isNumberString(const std::string &str)
{
    for (const auto &c : str)
    {
        if (!isdigit(c))
            return false;
    }
    return true;
}

bool seval_to_ccvaluemapintkey(const se::Value &v, cocos2d::ValueMapIntKey *ret)
{
    assert(ret != nullptr);
    if (v.isNullOrUndefined())
    {
        ret->clear();
        return true;
    }

    SE_PRECONDITION3(v.isObject(), false, ret->clear());
    SE_PRECONDITION3(!v.isNullOrUndefined(), false, ret->clear());

    se::Object *obj = v.toObject();

    cocos2d::ValueMapIntKey &dict = *ret;

    std::vector<std::string> allKeys;
    SE_PRECONDITION3(obj->getAllKeys(&allKeys), false, ret->clear());

    bool ok = false;
    se::Value value;
    cocos2d::Value ccvalue;
    for (const auto &key : allKeys)
    {
        SE_PRECONDITION3(obj->getProperty(key.c_str(), &value), false, ret->clear());

        if (!isNumberString(key))
        {
            SE_LOGD("seval_to_ccvaluemapintkey, found not numeric key: %s", key.c_str());
            continue;
        }

        int intKey = atoi(key.c_str());

        ok = seval_to_ccvalue(value, &ccvalue);
        SE_PRECONDITION3(ok, false, ret->clear());
        dict.emplace(intKey, ccvalue);
    }

    return true;
}

bool seval_to_ccvaluevector(const se::Value &v, cocos2d::ValueVector *ret)
{
    assert(ret != nullptr);

    SE_PRECONDITION3(v.isObject(), false, ret->clear());

    se::Object *obj = v.toObject();
    SE_PRECONDITION3(obj->isArray(), false, ret->clear());

    uint32_t len = 0;
    obj->getArrayLength(&len);

    bool ok = false;
    se::Value value;
    cocos2d::Value ccvalue;
    for (uint32_t i = 0; i < len; ++i)
    {
        if (obj->getArrayElement(i, &value))
        {
            ok = seval_to_ccvalue(value, &ccvalue);
            SE_PRECONDITION3(ok, false, ret->clear());
            ret->push_back(ccvalue);
        }
    }

    return true;
}

bool sevals_variadic_to_ccvaluevector(const se::ValueArray &args, cocos2d::ValueVector *ret)
{
    bool ok = false;
    cocos2d::Value ccvalue;

    for (const auto &arg : args)
    {
        ok = seval_to_ccvalue(arg, &ccvalue);
        SE_PRECONDITION3(ok, false, ret->clear());
        ret->push_back(ccvalue);
    }

    return true;
}

bool seval_to_blendfunc(const se::Value &v, cocos2d::BlendFunc *ret)
{
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to BlendFunc failed!");
    se::Object *obj = v.toObject();
    se::Value value;
    bool ok = false;

    ok = obj->getProperty("src", &value);
    SE_PRECONDITION3(ok, false, *ret = cocos2d::BlendFunc::DISABLE);
    ret->src = value.toUint32();
    ok = obj->getProperty("dst", &value);
    SE_PRECONDITION3(ok, false, *ret = cocos2d::BlendFunc::DISABLE);

    ret->dst = value.toUint32();
    return true;
}

bool seval_to_std_vector_string(const se::Value &v, std::vector<std::string> *ret)
{
    assert(ret != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to vector of String failed!");
    se::Object *obj = v.toObject();
    SE_PRECONDITION2(obj->isArray(), false, "Convert parameter to vector of String failed!");
    uint32_t len = 0;
    if (obj->getArrayLength(&len))
    {
        se::Value value;
        for (uint32_t i = 0; i < len; ++i)
        {
            SE_PRECONDITION3(obj->getArrayElement(i, &value) && value.isString(), false, ret->clear());
            ret->push_back(value.toString());
        }
        return true;
    }

    ret->clear();
    return true;
}

bool seval_to_std_vector_int(const se::Value &v, std::vector<int> *ret)
{
    assert(ret != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to vector of int failed!");
    se::Object *obj = v.toObject();

    if (obj->isArray())
    {
        uint32_t len = 0;
        if (obj->getArrayLength(&len))
        {
            se::Value value;
            for (uint32_t i = 0; i < len; ++i)
            {
                SE_PRECONDITION3(obj->getArrayElement(i, &value) && value.isNumber(), false, ret->clear());
                ret->push_back(value.toInt32());
            }
            return true;
        }
    }
    else if (obj->isTypedArray())
    {
        size_t bytesPerElements = 0;
        uint8_t *data = nullptr;
        size_t dataBytes = 0;
        se::Object::TypedArrayType type = obj->getTypedArrayType();

#define SE_UINT8_PTR_TO_INT(ptr) (*((uint8_t *)(ptr)))
#define SE_UINT16_PTR_TO_INT(ptr) (*((uint16_t *)(ptr)))
#define SE_UINT32_PTR_TO_INT(ptr) (*((uint32_t *)(ptr)))

        if (obj->getTypedArrayData(&data, &dataBytes))
        {
            for (size_t i = 0; i < dataBytes; i += bytesPerElements)
            {
                switch (type)
                {
                case se::Object::TypedArrayType::INT8:
                case se::Object::TypedArrayType::UINT8:
                case se::Object::TypedArrayType::UINT8_CLAMPED:
                    ret->push_back(SE_UINT8_PTR_TO_INT(data + i));
                    bytesPerElements = 1;
                    break;
                case se::Object::TypedArrayType::INT16:
                case se::Object::TypedArrayType::UINT16:
                    ret->push_back(SE_UINT16_PTR_TO_INT(data + i));
                    bytesPerElements = 2;
                    break;
                case se::Object::TypedArrayType::INT32:
                case se::Object::TypedArrayType::UINT32:
                    ret->push_back(SE_UINT32_PTR_TO_INT(data + i));
                    bytesPerElements = 4;
                    break;
                default:
                    SE_LOGE("Unsupported typed array: %d\n", (int)type);
                    assert(false);
                    break;
                }
            }
        }

#undef SE_UINT8_PTR_TO_INT
#undef SE_UINT16_PTR_TO_INT
#undef SE_UINT32_PTR_TO_INT

        return true;
    }
    else
    {
        assert(false);
    }

    ret->clear();
    return true;
}

bool seval_to_std_vector_uint16(const se::Value &v, std::vector<uint16_t> *ret)
{
    assert(ret != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to vector of uint16 failed!");
    se::Object *obj = v.toObject();

    if (obj->isArray())
    {
        uint32_t len = 0;
        if (obj->getArrayLength(&len))
        {
            se::Value value;
            for (uint32_t i = 0; i < len; ++i)
            {
                SE_PRECONDITION3(obj->getArrayElement(i, &value) && value.isNumber(), false, ret->clear());
                ret->push_back(value.toUint16());
            }
            return true;
        }
    }
    else if (obj->isTypedArray())
    {
        size_t bytesPerElements = 0;
        uint8_t *data = nullptr;
        size_t dataBytes = 0;
        se::Object::TypedArrayType type = obj->getTypedArrayType();

        if (obj->getTypedArrayData(&data, &dataBytes))
        {
            for (size_t i = 0; i < dataBytes; i += bytesPerElements)
            {
                switch (type)
                {
                case se::Object::TypedArrayType::INT16:
                case se::Object::TypedArrayType::UINT16:
                    ret->push_back(*((uint16_t *)(data + i)));
                    bytesPerElements = 2;
                    break;
                default:
                    SE_LOGE("Unsupported typed array: %d\n", (int)type);
                    assert(false);
                    break;
                }
            }
        }
        return true;
    }
    else
    {
        assert(false);
    }
    ret->clear();
    return true;
}

bool seval_to_std_vector_float(const se::Value &v, std::vector<float> *ret)
{
    assert(ret != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to vector of float failed!");
    se::Object *obj = v.toObject();
    SE_PRECONDITION2(obj->isArray(), false, "Convert parameter to vector of float failed!");
    uint32_t len = 0;
    if (obj->getArrayLength(&len))
    {
        se::Value value;
        for (uint32_t i = 0; i < len; ++i)
        {
            SE_PRECONDITION3(obj->getArrayElement(i, &value) && value.isNumber(), false, ret->clear());
            ret->push_back(value.toFloat());
        }
        return true;
    }

    ret->clear();
    return true;
}

bool seval_to_std_vector_Vec2(const se::Value &v, std::vector<cocos2d::Vec2> *ret)
{
    assert(ret != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to vector of Vec2 failed!");
    se::Object *obj = v.toObject();
    SE_PRECONDITION2(obj->isArray(), false, "Convert parameter to vector of Vec2 failed!");
    uint32_t len = 0;
    if (obj->getArrayLength(&len))
    {
        se::Value value;
        cocos2d::Vec2 pt;
        for (uint32_t i = 0; i < len; ++i)
        {
            SE_PRECONDITION3(obj->getArrayElement(i, &value) && seval_to_Vec2(value, &pt), false, ret->clear());
            ret->push_back(pt);
        }
        return true;
    }

    ret->clear();
    return true;
}

bool seval_to_std_map_string_string(const se::Value &v, std::map<std::string, std::string> *ret)
{
    assert(ret != nullptr);

    if (v.isNullOrUndefined())
    {
        ret->clear();
        return true;
    }

    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to map of String to String failed!");
    SE_PRECONDITION3(!v.isNullOrUndefined(), false, ret->clear());

    se::Object *obj = v.toObject();

    std::vector<std::string> allKeys;
    SE_PRECONDITION3(obj->getAllKeys(&allKeys), false, ret->clear());

    bool ok = false;
    se::Value value;
    std::string strValue;
    for (const auto &key : allKeys)
    {
        SE_PRECONDITION3(obj->getProperty(key.c_str(), &value), false, ret->clear());
        ok = seval_to_std_string(value, &strValue);
        SE_PRECONDITION3(ok, false, ret->clear());
        ret->emplace(key, strValue);
    }

    return true;
}

bool seval_to_Data(const se::Value &v, cocos2d::Data *ret)
{
    assert(ret != nullptr);
    SE_PRECONDITION2(v.isObject() && v.toObject()->isTypedArray(), false, "Convert parameter to Data failed!");
    uint8_t *ptr = nullptr;
    size_t length = 0;
    bool ok = v.toObject()->getTypedArrayData(&ptr, &length);
    if (ok)
    {
        ret->copy(ptr, length);
    }
    else
    {
        ret->clear();
    }

    return ok;
}

bool seval_to_DownloaderHints(const se::Value &v, cocos2d::network::DownloaderHints *ret)
{
    static cocos2d::network::DownloaderHints ZERO = {0, 0, ""};
    assert(ret != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to DownloaderHints failed!");
    se::Value tmp;
    se::Object *obj = v.toObject();
    bool ok = false;

    ok = obj->getProperty("countOfMaxProcessingTasks", &tmp);
    SE_PRECONDITION3(ok && tmp.isNumber(), false, *ret = ZERO);
    ret->countOfMaxProcessingTasks = tmp.toUint32();

    ok = obj->getProperty("timeoutInSeconds", &tmp);
    SE_PRECONDITION3(ok && tmp.isNumber(), false, *ret = ZERO);
    ret->timeoutInSeconds = tmp.toUint32();

    ok = obj->getProperty("tempFileNameSuffix", &tmp);
    SE_PRECONDITION3(ok && tmp.isString(), false, *ret = ZERO);
    ret->tempFileNameSuffix = tmp.toString();

    return ok;
}

#if USE_GFX_RENDERER
bool seval_to_Rect(const se::Value &v, cocos2d::renderer::Rect *rect)
{
    assert(rect != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to Rect failed!");
    se::Object *obj = v.toObject();
    se::Value x;
    se::Value y;
    se::Value width;
    se::Value height;

    bool ok = obj->getProperty("x", &x);
    SE_PRECONDITION3(ok && x.isNumber(), false, *rect = cocos2d::renderer::Rect::ZERO);
    ok = obj->getProperty("y", &y);
    SE_PRECONDITION3(ok && y.isNumber(), false, *rect = cocos2d::renderer::Rect::ZERO);
    ok = obj->getProperty("w", &width);
    SE_PRECONDITION3(ok && width.isNumber(), false, *rect = cocos2d::renderer::Rect::ZERO);
    ok = obj->getProperty("h", &height);
    SE_PRECONDITION3(ok && height.isNumber(), false, *rect = cocos2d::renderer::Rect::ZERO);
    rect->x = x.toFloat();
    rect->y = y.toFloat();
    rect->w = width.toFloat();
    rect->h = height.toFloat();

    return true;
}

bool seval_to_std_vector_Pass(const se::Value &v, cocos2d::Vector<cocos2d::renderer::Pass *> *ret)
{
    assert(ret != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to vector of Pass failed!");
    se::Object *obj = v.toObject();
    assert(obj->isArray());
    SE_PRECONDITION2(obj->isArray(), false, "Convert parameter to vector of Pass failed!");
    uint32_t len = 0;
    if (obj->getArrayLength(&len))
    {
        se::Value value;
        cocos2d::renderer::Pass *pt = nullptr;
        for (uint32_t i = 0; i < len; ++i)
        {
            SE_PRECONDITION3(obj->getArrayElement(i, &value), false, ret->clear());
            pt = static_cast<cocos2d::renderer::Pass *>(value.toObject()->getPrivateData());
            ret->pushBack(pt);
        }
        return true;
    }

    ret->clear();
    return true;
}

bool seval_to_std_vector_Texture(const se::Value &v, std::vector<cocos2d::renderer::Texture *> *ret)
{
    assert(ret != nullptr);
    SE_PRECONDITION2(v.isObject() && v.toObject()->isArray(), false, "Convert parameter to vector of Texture failed!");

    se::Object *obj = v.toObject();

    uint32_t len = 0;
    if (obj->getArrayLength(&len) && len > 0)
    {
        for (uint32_t i = 0; i < len; ++i)
        {
            se::Value textureVal;
            if (obj->getArrayElement(i, &textureVal) && textureVal.isObject())
            {
                cocos2d::renderer::Texture *texture = nullptr;
                seval_to_native_ptr(textureVal, &texture);
                ret->push_back(texture);
            }
        }
        return true;
    }

    ret->clear();
    return true;
}

bool seval_to_std_vector_RenderTarget(const se::Value &v, std::vector<cocos2d::renderer::RenderTarget *> *ret)
{
    assert(false);
    return true;
}

bool seval_to_TextureOptions(const se::Value &v, cocos2d::renderer::Texture::Options *ret)
{
    assert(ret != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to TextureOption failed!");
    se::Object *obj = v.toObject();
    se::Value images;
    if (obj->getProperty("images", &images) && images.isObject() && images.toObject()->isArray())
    {
        uint32_t len = 0;
        se::Object *arr = images.toObject();
        if (arr->getArrayLength(&len))
        {
            se::Value imageVal;
            for (uint32_t i = 0; i < len; ++i)
            {
                if (arr->getArrayElement(i, &imageVal))
                {
                    if (imageVal.isObject() && imageVal.toObject()->isTypedArray())
                    {
                        cocos2d::renderer::Texture::Image img;
                        imageVal.toObject()->getTypedArrayData(&img.data, &img.length);
                        ret->images.push_back(img);
                    }
                    else if (imageVal.isNull())
                    {
                        ret->images.push_back(cocos2d::renderer::Texture::Image());
                    }
                    else
                    {
                        SE_LOGE("Texture image isn't a typed array object or null!");
                        assert(false);
                    }
                }
            }
        }
    }

    se::Value tmp;
    if (obj->getProperty("mipmap", &tmp))
    {
        seval_to_boolean(tmp, &ret->hasMipmap);
    }

    if (obj->getProperty("width", &tmp))
    {
        seval_to_uint16(tmp, &ret->width);
    }

    if (obj->getProperty("height", &tmp))
    {
        seval_to_uint16(tmp, &ret->height);
    }

    if (obj->getProperty("glInternalFormat", &tmp))
    {
        seval_to_uint32(tmp, &ret->glInternalFormat);
    }

    if (obj->getProperty("glFormat", &tmp))
    {
        seval_to_uint32(tmp, &ret->glFormat);
    }

    if (obj->getProperty("glType", &tmp))
    {
        seval_to_uint32(tmp, &ret->glType);
    }

    if (obj->getProperty("anisotropy", &tmp))
    {
        seval_to_int32(tmp, &ret->anisotropy);
    }

    if (obj->getProperty("minFilter", &tmp))
    {
        seval_to_int8(tmp, (int8_t *)&ret->minFilter);
    }

    if (obj->getProperty("magFilter", &tmp))
    {
        seval_to_int8(tmp, (int8_t *)&ret->magFilter);
    }

    if (obj->getProperty("mipFilter", &tmp))
    {
        seval_to_int8(tmp, (int8_t *)&ret->mipFilter);
    }

    if (obj->getProperty("wrapS", &tmp))
    {
        seval_to_uint16(tmp, (uint16_t *)&ret->wrapS);
    }

    if (obj->getProperty("wrapT", &tmp))
    {
        seval_to_uint16(tmp, (uint16_t *)&ret->wrapT);
    }

    if (obj->getProperty("flipY", &tmp))
    {
        seval_to_boolean(tmp, &ret->flipY);
    }

    if (obj->getProperty("premultiplyAlpha", &tmp))
    {
        seval_to_boolean(tmp, &ret->premultiplyAlpha);
    }

    if (obj->getProperty("compressed", &tmp))
    {
        seval_to_boolean(tmp, &ret->compressed);
    }

    return true;
}

bool seval_to_TextureSubImageOption(const se::Value &v, cocos2d::renderer::Texture::SubImageOption *ret)
{
    assert(ret != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to TextureSubImageOption failed!");
    se::Object *obj = v.toObject();

    uint32_t *ptr = nullptr;
    size_t length = 0;
    obj->getTypedArrayData((uint8_t **)(&ptr), &length);

    *ret = cocos2d::renderer::Texture2D::SubImageOption(
        *ptr,       // x
        *(ptr + 1), // y
        *(ptr + 2), // width
        *(ptr + 3), // height
        *(ptr + 4), // level
        *(ptr + 5), // flipY
        *(ptr + 6)  // premultiplyAlpha
    );

    uint32_t imageDataLength = *(ptr + 7);
    ret->imageData = (uint8_t *)(ptr + 8);
    ret->imageDataLength = imageDataLength;
    return true;
}

bool seval_to_TextureImageOption(const se::Value &v, cocos2d::renderer::Texture::ImageOption *ret)
{
    assert(ret != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to TextureImageOption failed!");
    se::Object *obj = v.toObject();
    se::Value imageVal;
    if (obj->getProperty("image", &imageVal) && imageVal.isObject() && imageVal.toObject()->isTypedArray())
    {
        cocos2d::renderer::Texture::Image img;
        imageVal.toObject()->getTypedArrayData(&img.data, &img.length);
        ret->image = img;
    }

    se::Value tmp;

    if (obj->getProperty("width", &tmp))
    {
        seval_to_uint16(tmp, &ret->width);
    }

    if (obj->getProperty("level", &tmp))
    {
        seval_to_int32(tmp, &ret->level);
    }

    if (obj->getProperty("height", &tmp))
    {
        seval_to_uint16(tmp, &ret->height);
    }

    if (obj->getProperty("flipY", &tmp))
    {
        seval_to_boolean(tmp, &ret->flipY);
    }

    if (obj->getProperty("premultiplyAlpha", &tmp))
    {
        seval_to_boolean(tmp, &ret->premultiplyAlpha);
    }

    return true;
}

bool ccvaluevector_to_EffectPass(const cocos2d::ValueVector &v, cocos2d::Vector<cocos2d::renderer::Pass *> *ret)
{
    assert(ret != nullptr);

    for (const auto &value : v)
    {
        cocos2d::ValueMap valMap;
        valMap = value.asValueMap();
        cocos2d::renderer::Pass *cobj = new (std::nothrow) cocos2d::renderer::Pass(valMap["program"].asString());

        const auto &iter0 = valMap.find("rasterizerState");
        uint32_t cullMode = 0;
        if (iter0 != valMap.end())
        {
            const cocos2d::Value &val0 = iter0->second;
            const cocos2d::ValueMap &valMap0 = val0.asValueMap();
            const auto &it0 = valMap0.find("cullMode");

            if (it0 != valMap0.end())
            {
                cullMode = it0->second.asUnsignedInt();
            }
        }
        cobj->setCullMode(static_cast<cocos2d::renderer::CullMode>(cullMode));

        const auto &iter1 = valMap.find("blendState");
        cocos2d::renderer::BlendOp blendEq = cocos2d::renderer::BlendOp::ADD, blendAlphaEq = cocos2d::renderer::BlendOp::ADD;
        cocos2d::renderer::BlendFactor blendSrc = cocos2d::renderer::BlendFactor::SRC_ALPHA, blendDst = cocos2d::renderer::BlendFactor::ONE_MINUS_SRC_ALPHA, blendSrcAlpha = cocos2d::renderer::BlendFactor::SRC_ALPHA, blendDstAlpha = cocos2d::renderer::BlendFactor::ONE_MINUS_SRC_ALPHA;
        uint32_t blendColor = 0xffffffff;
        if (iter1 != valMap.end())
        {
            const cocos2d::Value &val1 = iter1->second;
            const cocos2d::ValueMap &valMap1 = val1.asValueMap();
            const auto &it1 = valMap1.find("targets");

            if (it1 != valMap1.end())
            {
                const cocos2d::ValueVector &vec1 = it1->second.asValueVector();

                for (const auto &e : vec1)
                {
                    cocos2d::ValueMap target = e.asValueMap();
                    if (target.find("blendEq") != target.end())
                    {
                        blendEq = static_cast<cocos2d::renderer::BlendOp>(target.at("blendEq").asUnsignedInt());
                    }

                    if (target.find("blendSrc") != target.end())
                    {
                        blendDst = static_cast<cocos2d::renderer::BlendFactor>(target.at("blendSrc").asUnsignedInt());
                    }

                    if (target.find("blendDst") != target.end())
                    {
                        blendDst = static_cast<cocos2d::renderer::BlendFactor>(target.at("blendDst").asUnsignedInt());
                    }

                    if (target.find("blendAlphaEq") != target.end())
                    {
                        blendAlphaEq = static_cast<cocos2d::renderer::BlendOp>(target.at("blendAlphaEq").asUnsignedInt());
                    }

                    if (target.find("blendSrcAlpha") != target.end())
                    {
                        blendSrcAlpha = static_cast<cocos2d::renderer::BlendFactor>(target.at("blendSrcAlpha").asUnsignedInt());
                    }

                    if (target.find("blendDstAlpha") != target.end())
                    {
                        blendDstAlpha = static_cast<cocos2d::renderer::BlendFactor>(target.at("blendDstAlpha").asUnsignedInt());
                    }

                    if (target.find("blendColor") != target.end())
                    {
                        blendColor = target.at("blendColor").asUnsignedInt();
                    }

                    break;
                }
            }

            cobj->setBlend(blendEq, blendSrc, blendDst, blendAlphaEq, blendSrcAlpha, blendDstAlpha, blendColor);
        }

        const auto &iter2 = valMap.find("depthStencilState");
        bool depthTest = false, depthWrite = false;
        cocos2d::renderer::DepthFunc depthFunc = cocos2d::renderer::DepthFunc::LESS;

        bool stencilTest = false;
        uint8_t stencilMaskFront = 0xff, stencilWriteMaskFront = 0xff, stencilMaskBack = 0xff, stencilWriteMaskBack = 0xff;
        uint32_t stencilRefFront = 0, stencilRefBack = 0;
        cocos2d::renderer::StencilFunc stencilFuncFront = cocos2d::renderer::StencilFunc::ALWAYS, stencilFuncBack = cocos2d::renderer::StencilFunc::ALWAYS;
        cocos2d::renderer::StencilOp stencilFailOpFront = cocos2d::renderer::StencilOp::KEEP, stencilFailOpBack = cocos2d::renderer::StencilOp::KEEP, stencilZFailOpFront = cocos2d::renderer::StencilOp::KEEP, stencilZFailOpBack = cocos2d::renderer::StencilOp::KEEP, stencilZPassOpFront = cocos2d::renderer::StencilOp::KEEP, stencilZPassOpBack = cocos2d::renderer::StencilOp::KEEP;

        if (iter2 != valMap.end())
        {
            const cocos2d::Value &val2 = iter2->second;
            cocos2d::ValueMap state = val2.asValueMap();

            if (state.find("depthTest") != state.end())
            {
                depthTest = state.at("depthTest").asBool();
            }

            if (state.find("depthWrite") != state.end())
            {
                depthWrite = state.at("depthWrite").asBool();
            }

            if (state.find("depthFunc") != state.end())
            {
                depthFunc = static_cast<cocos2d::renderer::DepthFunc>(state.at("depthFunc").asUnsignedInt());
            }

            if (state.find("stencilTest") != state.end())
            {
                stencilTest = state.at("stencilTest").asBool();
            }

            if (state.find("stencilFuncFront") != state.end())
            {
                stencilFuncFront = static_cast<cocos2d::renderer::StencilFunc>(state.at("stencilFuncFront").asUnsignedInt());
            }

            if (state.find("stencilRefFront") != state.end())
            {
                stencilRefFront = state.at("stencilRefFront").asUnsignedInt();
            }

            if (state.find("stencilMaskFront") != state.end())
            {
                stencilMaskFront = state.at("stencilMaskFront").asUnsignedInt();
            }

            if (state.find("stencilFailOpFront") != state.end())
            {
                stencilFailOpFront = static_cast<cocos2d::renderer::StencilOp>(state.at("stencilFailOpFront").asUnsignedInt());
            }

            if (state.find("stencilZFailOpFront") != state.end())
            {
                stencilZFailOpFront = static_cast<cocos2d::renderer::StencilOp>(state.at("stencilZFailOpFront").asUnsignedInt());
            }

            if (state.find("stencilZPassOpFront") != state.end())
            {
                stencilZPassOpFront = static_cast<cocos2d::renderer::StencilOp>(state.at("stencilZPassOpFront").asUnsignedInt());
            }

            if (state.find("stencilWriteMaskFront") != state.end())
            {
                stencilWriteMaskFront = state.at("stencilWriteMaskFront").asUnsignedInt();
            }

            if (state.find("stencilFuncBack") != state.end())
            {
                stencilFuncBack = static_cast<cocos2d::renderer::StencilFunc>(state.at("stencilFuncBack").asUnsignedInt());
            }

            if (state.find("stencilRefBack") != state.end())
            {
                stencilRefBack = state.at("stencilRefBack").asUnsignedInt();
            }

            if (state.find("stencilMaskBack") != state.end())
            {
                stencilMaskBack = state.at("stencilMaskBack").asUnsignedInt();
            }

            if (state.find("stencilFailOpBack") != state.end())
            {
                stencilFailOpBack = static_cast<cocos2d::renderer::StencilOp>(state.at("stencilFailOpBack").asUnsignedInt());
            }

            if (state.find("stencilZFailOpBack") != state.end())
            {
                stencilZFailOpBack = static_cast<cocos2d::renderer::StencilOp>(state.at("stencilZFailOpBack").asUnsignedInt());
            }

            if (state.find("stencilZPassOpBack") != state.end())
            {
                stencilZPassOpBack = static_cast<cocos2d::renderer::StencilOp>(state.at("stencilZPassOpBack").asUnsignedInt());
            }

            if (state.find("stencilWriteMaskBack") != state.end())
            {
                stencilWriteMaskBack = state.at("stencilWriteMaskBack").asUnsignedInt();
            }

            cobj->setDepth(depthTest, depthWrite, depthFunc); // depth func

            if (stencilTest)
            {
                cobj->setStencilFront(stencilFuncFront, stencilRefFront, stencilMaskFront, stencilFailOpFront, stencilZFailOpFront, stencilZPassOpFront, stencilWriteMaskFront);
                cobj->setStencilBack(stencilFuncBack, stencilRefBack, stencilMaskBack, stencilFailOpBack, stencilZFailOpBack, stencilZPassOpBack, stencilWriteMaskBack);
            }
        }

        cobj->autorelease();
        ret->pushBack(cobj);
    }

    return true;
}

bool seval_to_EffectTechnique(const se::Value &v, cocos2d::renderer::Technique **ret)
{
    SE_PRECONDITION2(v.isObject(), false, "Convert Effect Technique failed!");

    cocos2d::ValueMap valMap;
    if (seval_to_ccvaluemap(v, &valMap))
    {
        std::vector<std::string> stages;
        const auto &iter0 = valMap.find("stages");
        if (iter0 != valMap.end())
        {
            const cocos2d::Value &val = iter0->second;
            const cocos2d::ValueVector &vector = val.asValueVector();

            for (const auto &e : vector)
            {
                stages.push_back(e.asString());
            }
        }
        else
        {
            stages.push_back("opaque");
        }

        int layer = 0;
        const auto &iter1 = valMap.find("layer");
        if (iter1 != valMap.end())
        {
            const cocos2d::Value &val = iter1->second;
            layer = val.asInt();
        }

        cocos2d::Vector<cocos2d::renderer::Pass *> passes;
        const auto &iter2 = valMap.find("passes");
        if (iter2 != valMap.end())
        {
            const cocos2d::Value &val = iter2->second;
            const cocos2d::ValueVector &vector = val.asValueVector();
            ccvaluevector_to_EffectPass(vector, &passes);
        }

        *ret = new (std::nothrow) cocos2d::renderer::Technique(stages, passes, layer);

        return true;
    }

    return false;
}

bool seval_to_EffectAsset(const std::string &e, cocos2d::Vector<cocos2d::renderer::Technique *> *ret)
{
    se::Object *asset = se::Object::createJSONObject(e);
    asset = se::Object::createJSONObject(e);
    se::Value techniques;
    asset->getProperty("techniques", &techniques);
    se::Object *techs = techniques.toObject();
    bool ok = techs->isArray();
    SE_PRECONDITION2(ok, false, "Convert Effect Asset Failed!");

    uint32_t len = 0;
    techs->getArrayLength(&len);
    for (uint32_t i = 0; i < len; ++i)
    {
        se::Value val;
        if (techs->getArrayElement(i, &val) && val.isObject())
        {
            cocos2d::renderer::Technique *tech = nullptr;
            ok &= seval_to_EffectTechnique(val, &tech);
            SE_PRECONDITION2(ok, false, "Effect Technique Create Failed!");
            ret->pushBack(tech);
        }
    }
    return true;
}

bool seval_to_EffectProperty(const se::Value &v, std::unordered_map<std::string, cocos2d::renderer::Effect::Property> *ret)
{
    assert(ret != nullptr);
    if (v.isNullOrUndefined())
    {
        ret->clear();
        return true;
    }

    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to EffectProperty failed!");

    se::Object *obj = v.toObject();
    std::vector<std::string> keys;
    obj->getAllKeys(&keys);

    for (const auto &key : keys)
    {
        se::Value value;
        if (obj->getProperty(key.c_str(), &value) && value.isObject())
        {
            cocos2d::renderer::Technique::Parameter property;
            seval_to_TechniqueParameter(value, &property);
            ret->emplace(key, property);
        }
    }

    return true;
}

bool seval_to_EffectDefineTemplate(const se::Value &v, std::vector<cocos2d::ValueMap> *ret)
{
    assert(ret != nullptr);
    bool ok = v.isObject() && v.toObject()->isArray();
    SE_PRECONDITION2(ok, false, "Convert parameter to EffectDefineTemplate failed!");

    se::Object *obj = v.toObject();
    uint32_t len = 0;
    obj->getArrayLength(&len);
    for (uint32_t i = 0; i < len; ++i)
    {
        se::Value value;
        cocos2d::ValueMap valMap;
        if (obj->getArrayElement(i, &value) && value.isObject())
        {
            if (seval_to_ccvaluemap(value, &valMap))
            {
                ret->push_back(std::move(valMap));
            }
        }
    }

    return true;
}

bool seval_to_TechniqueParameter_not_constructor(const se::Value &v, cocos2d::renderer::Technique::Parameter *ret, bool directly)
{
    assert(ret != nullptr);
    auto paramType = ret->getType();
    switch (paramType)
    {
    case cocos2d::renderer::Technique::Parameter::Type::INT:
    {
        int32_t value;
        seval_to_int32(v, &value);
        cocos2d::renderer::Technique::Parameter param(ret->getName(), paramType, &value);
        *ret = std::move(param);
        break;
    }
    case cocos2d::renderer::Technique::Parameter::Type::INT2:
    case cocos2d::renderer::Technique::Parameter::Type::INT3:
    case cocos2d::renderer::Technique::Parameter::Type::INT4:
    {
        se::Object *obj = v.toObject();
        SE_PRECONDITION2(obj->isTypedArray(), false, "Convert parameter to float array failed!");
        uint8_t *data = nullptr;
        size_t len = 0;
        obj->getTypedArrayData(&data, &len);
        uint8_t el = cocos2d::renderer::Technique::Parameter::getElements(paramType);
        uint8_t count = (len / sizeof(int)) / el;
        cocos2d::renderer::Technique::Parameter param(ret->getName(), paramType, (int *)data, count);
        *ret = std::move(param);
        break;
    }
    case cocos2d::renderer::Technique::Parameter::Type::FLOAT:
    {
        float value;
        seval_to_float(v, &value);
        cocos2d::renderer::Technique::Parameter param(ret->getName(), paramType, &value);
        *ret = std::move(param);
        break;
    }
    case cocos2d::renderer::Technique::Parameter::Type::FLOAT2:
    case cocos2d::renderer::Technique::Parameter::Type::FLOAT3:
    case cocos2d::renderer::Technique::Parameter::Type::FLOAT4:
    case cocos2d::renderer::Technique::Parameter::Type::MAT4:
    case cocos2d::renderer::Technique::Parameter::Type::MAT3:
    case cocos2d::renderer::Technique::Parameter::Type::MAT2:
    case cocos2d::renderer::Technique::Parameter::Type::COLOR3:
    case cocos2d::renderer::Technique::Parameter::Type::COLOR4:
    {
        se::Object *obj = v.toObject();
        SE_PRECONDITION2(obj->isTypedArray(), false, "Convert parameter to float array failed!");

        if (directly)
        {
            cocos2d::renderer::Technique::Parameter param(ret->getName(), paramType, obj);
            *ret = std::move(param);
        }
        else
        {
            uint8_t *data = nullptr;
            size_t len = 0;
            obj->getTypedArrayData(&data, &len);
            uint8_t el = cocos2d::renderer::Technique::Parameter::getElements(paramType);
            uint8_t count = (len / sizeof(float)) / el;
            cocos2d::renderer::Technique::Parameter param(ret->getName(), paramType, (float *)data, count);
            *ret = std::move(param);
        }

        break;
    }
    case cocos2d::renderer::Technique::Parameter::Type::TEXTURE_2D:
    case cocos2d::renderer::Technique::Parameter::Type::TEXTURE_CUBE:
    {
        se::Object *obj = v.toObject();
        if (obj->isArray())
        {
            uint32_t arrLen = 0;
            obj->getArrayLength(&arrLen);
            if (arrLen == 1)
            {
                cocos2d::renderer::Texture *texture = nullptr;
                seval_to_native_ptr(v, &texture);
                cocos2d::renderer::Technique::Parameter param(ret->getName(), paramType, texture);
                *ret = std::move(param);
            }
            else
            {
                std::vector<cocos2d::renderer::Texture *> textures;
                for (uint32_t i = 0; i < arrLen; ++i)
                {
                    se::Value texVal;
                    obj->getArrayElement(i, &texVal);
                    cocos2d::renderer::Texture *tmpTex = nullptr;
                    seval_to_native_ptr(texVal, &tmpTex);
                    textures.push_back(tmpTex);
                }
                cocos2d::renderer::Technique::Parameter param(ret->getName(), paramType, textures);
                *ret = std::move(param);
            }
        }
        else
        {
            cocos2d::renderer::Texture *texture = nullptr;
            seval_to_native_ptr(v, &texture);
            cocos2d::renderer::Technique::Parameter param(ret->getName(), paramType, texture);
            *ret = std::move(param);
        }
        break;
    }
    default:
        assert(false);
        break;
    }

    return true;
}

bool seval_to_TechniqueParameter(const se::Value &v, cocos2d::renderer::Technique::Parameter *ret)
{
    assert(ret != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to TechniqueParameter failed!");
    se::Object *obj = v.toObject();
    se::Value tmp;
    std::string name;
    uint8_t size = 0;
    size_t len = 0;
    double number = 0.0;
    void *value = nullptr;
    cocos2d::renderer::Technique::Parameter::Type type = cocos2d::renderer::Technique::Parameter::Type::UNKNOWN;
    std::vector<cocos2d::renderer::Texture *> textures;
    cocos2d::renderer::Texture *texture = nullptr;

    bool ok = false;

    if (obj->getProperty("updateSubImage", &tmp))
    {
        type = cocos2d::renderer::Technique::Parameter::Type::TEXTURE_2D;
        size = 1;
        seval_to_native_ptr(v, &texture);
    }
    else
    {
        if (obj->getProperty("name", &tmp))
        {
            ok = seval_to_std_string(tmp, &name);
            SE_PRECONDITION2(ok, false, "Convert Parameter name failed!");
        }

        if (obj->getProperty("type", &tmp))
        {
            uint8_t typeValue = 0;
            ok = seval_to_uint8(tmp, &typeValue);
            SE_PRECONDITION2(ok, false, "Convert Parameter type failed!");
            type = (cocos2d::renderer::Technique::Parameter::Type)typeValue;
        }

        if (obj->getProperty("size", &tmp))
        {
            ok = seval_to_uint8(tmp, &size);
            SE_PRECONDITION2(ok, false, "Convert Parameter size failed!");
        }

        if (obj->getProperty("value", &tmp))
        {
            if (tmp.isNumber())
            {
                number = tmp.toNumber();
            }
            else if (tmp.isObject())
            {
                se::Object *valObj = tmp.toObject();
                if (valObj->isArray())
                {
                    ok = (type == cocos2d::renderer::Technique::Parameter::Type::TEXTURE_2D ||
                          type == cocos2d::renderer::Technique::Parameter::Type::TEXTURE_CUBE);
                    SE_PRECONDITION2(ok, false, "Convert Parameter val failed!");

                    uint32_t arrLen = 0;
                    valObj->getArrayLength(&arrLen);
                    for (uint32_t i = 0; i < arrLen; ++i)
                    {
                        se::Value texVal;
                        valObj->getArrayElement(i, &texVal);
                        cocos2d::renderer::Texture *tmpTex = nullptr;
                        seval_to_native_ptr(texVal, &tmpTex);
                        textures.push_back(tmpTex);
                    }
                }
                else if (valObj->isTypedArray())
                {
                    uint8_t *data = nullptr;
                    if (valObj->getTypedArrayData(&data, &len))
                    {
                        value = data;
                    }
                }
                else if (valObj->isArrayBuffer())
                {
                    uint8_t *data = nullptr;
                    if (valObj->getArrayBufferData(&data, &len))
                    {
                        value = data;
                    }
                }
                else
                {
                    ok = (type == cocos2d::renderer::Technique::Parameter::Type::TEXTURE_2D ||
                          type == cocos2d::renderer::Technique::Parameter::Type::TEXTURE_CUBE);

                    if (ok)
                    {
                        seval_to_native_ptr(tmp, &texture);
                    }
                }
            }
            else
            {
            }
        }
    }

    switch (type)
    {
    case cocos2d::renderer::Technique::Parameter::Type::INT:
    case cocos2d::renderer::Technique::Parameter::Type::INT2:
    case cocos2d::renderer::Technique::Parameter::Type::INT3:
    case cocos2d::renderer::Technique::Parameter::Type::INT4:
    {
        if (size == 1)
        {
            int intVal = (int)number;
            cocos2d::renderer::Technique::Parameter param(name, type, &intVal, 1);
            *ret = std::move(param);
        }
        else
        {
            uint8_t el = cocos2d::renderer::Technique::Parameter::getElements(type);
            uint8_t count = (len / sizeof(float)) / el;
            cocos2d::renderer::Technique::Parameter param(name, type, (int *)value, count);
            *ret = std::move(param);
        }
        break;
    }
    case cocos2d::renderer::Technique::Parameter::Type::FLOAT:
    case cocos2d::renderer::Technique::Parameter::Type::FLOAT2:
    case cocos2d::renderer::Technique::Parameter::Type::FLOAT3:
    case cocos2d::renderer::Technique::Parameter::Type::FLOAT4:
    case cocos2d::renderer::Technique::Parameter::Type::COLOR3:
    case cocos2d::renderer::Technique::Parameter::Type::COLOR4:
    case cocos2d::renderer::Technique::Parameter::Type::MAT2:
    case cocos2d::renderer::Technique::Parameter::Type::MAT3:
    case cocos2d::renderer::Technique::Parameter::Type::MAT4:
    {
        if (size == 1)
        {
            float floatVal = (float)number;
            cocos2d::renderer::Technique::Parameter param(name, type, &floatVal, 1);
            *ret = std::move(param);
        }
        else
        {
            uint8_t el = cocos2d::renderer::Technique::Parameter::getElements(type);
            uint8_t count = (len / sizeof(float)) / el;
            cocos2d::renderer::Technique::Parameter param(name, type, (float *)value, count);
            *ret = std::move(param);
        }
        break;
    }

    case cocos2d::renderer::Technique::Parameter::Type::TEXTURE_2D:
    case cocos2d::renderer::Technique::Parameter::Type::TEXTURE_CUBE:
    {
        if (size == 1)
        {
            cocos2d::renderer::Technique::Parameter param(name, type, texture);
            *ret = std::move(param);
        }
        else
        {
            cocos2d::renderer::Technique::Parameter param(name, type, textures);
            *ret = std::move(param);
        }
        break;
    }
    default:
        assert(false);
        break;
    }

    return true;
}

bool seval_to_std_vector_TechniqueParameter(const se::Value &v, std::vector<cocos2d::renderer::Technique::Parameter> *ret)
{
    assert(ret != nullptr);
    if (v.isNullOrUndefined())
    {
        ret->clear();
        return true;
    }
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to vector of TechniqueParameter failed!");

    se::Object *obj = v.toObject();
    uint32_t len = 0;
    obj->getArrayLength(&len);
    ret->reserve(len);
    for (uint32_t i = 0; i < len; ++i)
    {
        se::Value data;
        if (obj->getArrayElement(i, &data))
        {
            cocos2d::renderer::Technique::Parameter parameter;
            seval_to_TechniqueParameter(data, &parameter);
            ret->push_back(std::move(parameter));
        }
    }

    return true;
}

namespace
{
    void adjustShaderSource(std::string &shaderSource)
    {
#if CC_TARGET_PLATFORM == CC_PLATFORM_MAC || CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
        shaderSource = std::regex_replace(shaderSource, std::regex("precision\\s+(lowp|mediump|highp)\\s+float\\s*?;"), "");
        shaderSource = std::regex_replace(shaderSource, std::regex("\\s(lowp|mediump|highp)\\s"), " ");
#endif
    }
}

bool seval_to_ProgramLib_Template(const se::Value &v, cocos2d::renderer::ProgramLib::Template *ret)
{
    assert(ret != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to ProgramLib Template failed!");
    se::Object *obj = v.toObject();

    bool ok = false;
    se::Value tmp;

    if (obj->getProperty("id", &tmp))
    {
        ok = seval_to_uint32(tmp, &ret->id);
        SE_PRECONDITION2(ok, false, "Convert id failed!");
    }

    if (obj->getProperty("name", &tmp))
    {
        ok = seval_to_std_string(tmp, &ret->name);
        SE_PRECONDITION2(ok, false, "Convert name failed!");
    }

    if (obj->getProperty("vert", &tmp))
    {
        ok = seval_to_std_string(tmp, &ret->vert);
        SE_PRECONDITION2(ok, false, "Convert vert failed!");
        adjustShaderSource(ret->vert);
    }

    if (obj->getProperty("frag", &tmp))
    {
        ok = seval_to_std_string(tmp, &ret->frag);
        SE_PRECONDITION2(ok, false, "Convert frag failed!");
        adjustShaderSource(ret->frag);
    }

    if (obj->getProperty("defines", &tmp))
    {
        ok = seval_to_ccvaluevector(tmp, &ret->defines);
        SE_PRECONDITION2(ok, false, "Convert defines failed!");
    }

    return true;
}

bool seval_to_std_vector_ProgramLib_Template(const se::Value &v, std::vector<cocos2d::renderer::ProgramLib::Template> *ret)
{
    assert(ret != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to vector of ProgramLib Template failed!");

    se::Object *obj = v.toObject();
    uint32_t len = 0;
    obj->getArrayLength(&len);
    ret->reserve(len);
    for (uint32_t i = 0; i < len; ++i)
    {
        se::Value data;
        if (obj->getArrayElement(i, &data))
        {
            cocos2d::renderer::ProgramLib::Template parameter;
            if (seval_to_ProgramLib_Template(data, &parameter))
            {
                ret->push_back(std::move(parameter));
            }
        }
    }

    return true;
}
#endif // USE_GFX_RENDERER > 0

bool int32_to_seval(int32_t v, se::Value *ret)
{
    ret->setInt32(v);
    return true;
}

bool uint32_to_seval(uint32_t v, se::Value *ret)
{
    ret->setUint32(v);
    return true;
}

bool int16_to_seval(uint16_t v, se::Value *ret)
{
    ret->setInt16(v);
    return true;
}

bool uint16_to_seval(uint16_t v, se::Value *ret)
{
    ret->setUint16(v);
    return true;
}

bool int8_to_seval(int8_t v, se::Value *ret)
{
    ret->setInt8(v);
    return true;
}

bool uint8_to_seval(uint8_t v, se::Value *ret)
{
    ret->setUint8(v);
    return true;
}

bool boolean_to_seval(bool v, se::Value *ret)
{
    ret->setBoolean(v);
    return true;
}

bool float_to_seval(float v, se::Value *ret)
{
    ret->setFloat(v);
    return true;
}

bool double_to_seval(double v, se::Value *ret)
{
    ret->setNumber(v);
    return true;
}

bool long_to_seval(long v, se::Value *ret)
{
    ret->setLong(v);
    return true;
}

bool ulong_to_seval(unsigned long v, se::Value *ret)
{
    ret->setUlong(v);
    return true;
}

bool longlong_to_seval(long long v, se::Value *ret)
{
    ret->setLong((long)v);
    return true;
}

bool ssize_to_seval(ssize_t v, se::Value *ret)
{
    ret->setLong((long)v);
    return true;
}

bool size_to_seval(size_t v, se::Value *ret)
{
    ret->setLong((unsigned long)v);
    return true;
}

bool std_string_to_seval(const std::string &v, se::Value *ret)
{
    ret->setString(v);
    return true;
}

bool Vec2_to_seval(const cocos2d::Vec2 &v, se::Value *ret)
{
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("x", se::Value(v.x));
    obj->setProperty("y", se::Value(v.y));
    ret->setObject(obj);

    return true;
}

bool Vec3_to_seval(const cocos2d::Vec3 &v, se::Value *ret)
{
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("x", se::Value(v.x));
    obj->setProperty("y", se::Value(v.y));
    obj->setProperty("z", se::Value(v.z));
    ret->setObject(obj);

    return true;
}

bool Vec4_to_seval(const cocos2d::Vec4 &v, se::Value *ret)
{
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("x", se::Value(v.x));
    obj->setProperty("y", se::Value(v.y));
    obj->setProperty("z", se::Value(v.z));
    obj->setProperty("w", se::Value(v.w));
    ret->setObject(obj);

    return true;
}

bool Mat4_to_seval(const cocos2d::Mat4 &v, se::Value *ret)
{
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createArrayObject(16));

    for (uint8_t i = 0; i < 16; ++i)
    {
        obj->setArrayElement(i, se::Value(v.m[i]));
    }

    ret->setObject(obj);
    return true;
}

bool Size_to_seval(const cocos2d::Size &v, se::Value *ret)
{
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("width", se::Value(v.width));
    obj->setProperty("height", se::Value(v.height));
    ret->setObject(obj);
    return true;
}

bool Rect_to_seval(const cocos2d::Rect &v, se::Value *ret)
{
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("x", se::Value(v.origin.x));
    obj->setProperty("y", se::Value(v.origin.y));
    obj->setProperty("width", se::Value(v.size.width));
    obj->setProperty("height", se::Value(v.size.height));
    ret->setObject(obj);

    return true;
}

bool Color3B_to_seval(const cocos2d::Color3B &v, se::Value *ret)
{
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("r", se::Value(v.r));
    obj->setProperty("g", se::Value(v.g));
    obj->setProperty("b", se::Value(v.b));
    obj->setProperty("a", se::Value(255));
    ret->setObject(obj);

    return true;
}

bool Color4B_to_seval(const cocos2d::Color4B &v, se::Value *ret)
{
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("r", se::Value(v.r));
    obj->setProperty("g", se::Value(v.g));
    obj->setProperty("b", se::Value(v.b));
    obj->setProperty("a", se::Value(v.a));
    ret->setObject(obj);

    return true;
}

bool Color4F_to_seval(const cocos2d::Color4F &v, se::Value *ret)
{
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("r", se::Value(v.r));
    obj->setProperty("g", se::Value(v.g));
    obj->setProperty("b", se::Value(v.b));
    obj->setProperty("a", se::Value(v.a));
    ret->setObject(obj);

    return true;
}

bool Color3F_to_seval(const cocos2d::Color3F &v, se::Value *ret)
{
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("r", se::Value(v.r));
    obj->setProperty("g", se::Value(v.g));
    obj->setProperty("b", se::Value(v.b));
    ret->setObject(obj);
    return true;
}

bool ccvalue_to_seval(const cocos2d::Value &v, se::Value *ret)
{
    assert(ret != nullptr);
    bool ok = true;
    switch (v.getType())
    {
    case cocos2d::Value::Type::NONE:
        ret->setNull();
        break;
    case cocos2d::Value::Type::UNSIGNED:
        ret->setUint32(v.asUnsignedInt());
        break;
    case cocos2d::Value::Type::BOOLEAN:
        ret->setBoolean(v.asBool());
        break;
    case cocos2d::Value::Type::FLOAT:
    case cocos2d::Value::Type::DOUBLE:
        ret->setNumber(v.asDouble());
        break;
    case cocos2d::Value::Type::INTEGER:
        ret->setInt32(v.asInt());
        break;
    case cocos2d::Value::Type::STRING:
        ret->setString(v.asString());
        break;
    case cocos2d::Value::Type::VECTOR:
        ok = ccvaluevector_to_seval(v.asValueVector(), ret);
        break;
    case cocos2d::Value::Type::MAP:
        ok = ccvaluemap_to_seval(v.asValueMap(), ret);
        break;
    case cocos2d::Value::Type::INT_KEY_MAP:
        ok = ccvaluemapintkey_to_seval(v.asIntKeyMap(), ret);
        break;
    default:
        SE_LOGE("Could not the way to convert cocos2d::Value::Type (%d) type!", (int)v.getType());
        ok = false;
        break;
    }

    return ok;
}

bool ccvaluemap_to_seval(const cocos2d::ValueMap &v, se::Value *ret)
{
    assert(ret != nullptr);

    se::HandleObject obj(se::Object::createPlainObject());
    bool ok = true;
    for (const auto &e : v)
    {
        const std::string &key = e.first;
        const cocos2d::Value &value = e.second;

        if (key.empty())
            continue;

        se::Value tmp;
        if (!ccvalue_to_seval(value, &tmp))
        {
            ok = false;
            ret->setUndefined();
            break;
        }

        obj->setProperty(key.c_str(), tmp);
    }
    if (ok)
        ret->setObject(obj);

    return ok;
}

bool ccvaluemapintkey_to_seval(const cocos2d::ValueMapIntKey &v, se::Value *ret)
{
    assert(ret != nullptr);

    se::HandleObject obj(se::Object::createPlainObject());
    bool ok = true;
    for (const auto &e : v)
    {
        std::stringstream keyss;
        keyss << e.first;
        std::string key = keyss.str();
        const cocos2d::Value &value = e.second;

        if (key.empty())
            continue;

        se::Value tmp;
        if (!ccvalue_to_seval(value, &tmp))
        {
            ok = false;
            ret->setUndefined();
            break;
        }

        obj->setProperty(key.c_str(), tmp);
    }
    if (ok)
        ret->setObject(obj);

    return ok;
}

bool ccvaluevector_to_seval(const cocos2d::ValueVector &v, se::Value *ret)
{
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createArrayObject(v.size()));
    bool ok = true;

    uint32_t i = 0;
    for (const auto &value : v)
    {
        se::Value tmp;
        if (!ccvalue_to_seval(value, &tmp))
        {
            ok = false;
            ret->setUndefined();
            break;
        }

        obj->setArrayElement(i, tmp);
        ++i;
    }
    if (ok)
        ret->setObject(obj);

    return ok;
}

bool blendfunc_to_seval(const cocos2d::BlendFunc &v, se::Value *ret)
{
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("src", se::Value(v.src));
    obj->setProperty("dst", se::Value(v.dst));
    ret->setObject(obj);

    return true;
}

bool std_vector_domain_to_seval(const std::vector<cocos2d::network::NetDoctor::Domain> &v, se::Value *ret)
{
    assert(ret != nullptr);
    bool ok = true;
    if (!v.empty())
    {
        se::HandleObject obj(se::Object::createPlainObject());

        for (const auto &e : v)
        {
            const std::string &domain = e.domain;
            const long &latency = e.latency;

            se::Value se_value = se::Value(latency);
            if (!obj->setProperty(domain.c_str(), se_value))
            {
                ok = false;
                ret->setUndefined();
                break;
            }
        }
        if (ok)
        {
            ret->setObject(obj);
        }
    }
    return ok;
}

bool std_map_string_std_vector_domain_to_seval(const std::map<std::string, std::vector<cocos2d::network::NetDoctor::Domain>> &v, se::Value *ret)
{
    assert(ret != nullptr);
    bool ok = true;
    if (!v.empty())
    {
        se::HandleObject obj(se::Object::createPlainObject());

        for (const auto &e : v)
        {
            const std::string &gameId = e.first;
            const std::vector<cocos2d::network::NetDoctor::Domain> domains = e.second;

            se::Value se_value;
            if (!std_vector_domain_to_seval(domains, &se_value))
            {
                ok = false;
                ret->setUndefined();
                break;
            }

            if (!obj->setProperty(gameId.c_str(), se_value))
            {
                ok = false;
                ret->setUndefined();
                break;
            }
        }
        if (ok)
        {
            ret->setObject(obj);
        }
    }
    return ok;
}

namespace
{

    template <typename T>
    bool std_vector_T_to_seval(const std::vector<T> &v, se::Value *ret)
    {
        assert(ret != nullptr);
        se::HandleObject obj(se::Object::createArrayObject(v.size()));
        bool ok = true;

        uint32_t i = 0;
        for (const auto &value : v)
        {
            if (!obj->setArrayElement(i, se::Value(value)))
            {
                ok = false;
                ret->setUndefined();
                break;
            }
            ++i;
        }

        if (ok)
            ret->setObject(obj);

        return ok;
    }

    template <typename K, typename V>
    bool std_map_K_V_to_seval(const std::map<K, V> &v, se::Value *ret)
    {
        assert(ret != nullptr);
        bool ok = true;
        if (!v.empty())
        {
            se::HandleObject obj(se::Object::createPlainObject());

            for (const auto &e : v)
            {
                const K &key = e.first;
                const V &value = e.second;

                se::Value se_value = se::Value(value);
                if (!obj->setProperty(key.c_str(), se_value))
                {
                    ok = false;
                    ret->setUndefined();
                    break;
                }
            }
            if (ok)
            {
                ret->setObject(obj);
            }
        }
        return ok;
    }

    template <typename K1, typename K2, typename V>
    bool std_map_K_map_K_V_to_seval(const std::map<K1, std::map<K2, V>> &v, se::Value *ret)
    {
        assert(ret != nullptr);

        bool ok = true;
        if (!v.empty())
        {
            se::HandleObject obj(se::Object::createPlainObject());

            for (const auto &e : v)
            {
                const K1 &key = e.first;
                const std::map<K2, V> &value = e.second;
                se::Value se_value;
                if (!std_map_K_V_to_seval(value, &se_value))
                {
                    ok = false;
                    ret->setUndefined();
                    break;
                }
                if (!obj->setProperty(key.c_str(), se_value))
                {
                    ok = false;
                    ret->setUndefined();
                    break;
                }
            }

            if (ok)
                ret->setObject(obj);
        }

        return ok;
    }

}

bool std_vector_string_to_seval(const std::vector<std::string> &v, se::Value *ret)
{
    return std_vector_T_to_seval(v, ret);
}

bool std_vector_int_to_seval(const std::vector<int> &v, se::Value *ret)
{
    return std_vector_T_to_seval(v, ret);
}

bool std_vector_uint16_to_seval(const std::vector<uint16_t> &v, se::Value *ret)
{
    return std_vector_T_to_seval(v, ret);
}

bool std_vector_float_to_seval(const std::vector<float> &v, se::Value *ret)
{
    return std_vector_T_to_seval(v, ret);
}

bool std_map_string_long_to_seval(const std::map<std::string, long> &v, se::Value *ret)
{
    return std_map_K_V_to_seval(v, ret);
}

bool std_map_string_map_string_long_to_seval(const std::map<std::string, std::map<std::string, long>> &v, se::Value *ret)
{
    return std_map_K_map_K_V_to_seval(v, ret);
}

bool std_map_string_string_to_seval(const std::map<std::string, std::string> &v, se::Value *ret)
{
    assert(ret != nullptr);

    se::HandleObject obj(se::Object::createPlainObject());
    bool ok = true;
    for (const auto &e : v)
    {
        const std::string &key = e.first;
        const std::string &value = e.second;

        if (key.empty())
            continue;

        se::Value tmp;
        if (!std_string_to_seval(value, &tmp))
        {
            ok = false;
            ret->setUndefined();
            break;
        }

        obj->setProperty(key.c_str(), tmp);
    }

    if (ok)
        ret->setObject(obj);

    return ok;
}

namespace
{
    enum class DataType
    {
        INT,
        FLOAT
    };

    void toVec2(void *data, DataType type, se::Value *ret)
    {
        int32_t *intptr = (int32_t *)data;
        float *floatptr = (float *)data;
        cocos2d::Vec2 vec2;
        if (DataType::INT == type)
        {
            vec2.x = *intptr;
            vec2.y = *(intptr + 1);
        }
        else
        {
            vec2.x = *floatptr;
            vec2.y = *(floatptr + 1);
        }

        Vec2_to_seval(vec2, ret);
    }

    void toVec3(void *data, DataType type, se::Value *ret)
    {
        int32_t *intptr = (int32_t *)data;
        float *floatptr = (float *)data;
        cocos2d::Vec3 vec3;
        if (DataType::INT == type)
        {
            vec3.x = *intptr;
            vec3.y = *(intptr + 1);
            vec3.z = *(intptr + 2);
        }
        else
        {
            vec3.x = *floatptr;
            vec3.y = *(floatptr + 1);
            vec3.z = *(floatptr + 2);
        }

        Vec3_to_seval(vec3, ret);
    }

    void toVec4(void *data, DataType type, se::Value *ret)
    {
        int32_t *intptr = (int32_t *)data;
        float *floatptr = (float *)data;
        cocos2d::Vec4 vec4;
        if (DataType::INT == type)
        {
            vec4.x = *intptr;
            vec4.y = *(intptr + 1);
            vec4.z = *(intptr + 2);
            vec4.w = *(intptr + 3);
        }
        else
        {
            vec4.x = *floatptr;
            vec4.y = *(floatptr + 1);
            vec4.z = *(floatptr + 2);
            vec4.w = *(floatptr + 3);
        }

        Vec4_to_seval(vec4, ret);
    }

    void toMat(float *data, int num, se::Value *ret)
    {
        se::HandleObject obj(se::Object::createPlainObject());

        char propName[4] = {0};
        for (int i = 0; i < num; ++i)
        {
            if (i < 10)
                snprintf(propName, 3, "m0%d", i);
            else
                snprintf(propName, 3, "m%d", i);

            obj->setProperty(propName, se::Value(*(data + i)));
        }
        ret->setObject(obj);
    }
}

bool ManifestAsset_to_seval(const cocos2d::extension::ManifestAsset &v, se::Value *ret)
{
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("md5", se::Value(v.md5));
    obj->setProperty("path", se::Value(v.path));
    obj->setProperty("compressed", se::Value(v.compressed));
    obj->setProperty("size", se::Value(v.size));
    obj->setProperty("downloadState", se::Value(v.downloadState));
    ret->setObject(obj);

    return true;
}

bool Data_to_seval(const cocos2d::Data &v, se::Value *ret)
{
    assert(ret != nullptr);
    if (v.isNull())
    {
        ret->setNull();
    }
    else
    {
        se::HandleObject obj(se::Object::createTypedArray(se::Object::TypedArrayType::UINT8, v.getBytes(), v.getSize()));
        ret->setObject(obj, true);
    }
    return true;
}

bool DownloadTask_to_seval(const cocos2d::network::DownloadTask &v, se::Value *ret)
{
    assert(ret != nullptr);

    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("identifier", se::Value(v.identifier));
    obj->setProperty("requestURL", se::Value(v.requestURL));
    obj->setProperty("storagePath", se::Value(v.storagePath));
    ret->setObject(obj);

    return true;
}
bool std_vector_EffectDefine_to_seval(const std::vector<cocos2d::ValueMap> &v, se::Value *ret)
{
    assert(ret != nullptr);
    se::HandleObject arr(se::Object::createArrayObject(v.size()));
    ret->setObject(arr);

    uint32_t i = 0;
    for (const auto &valueMap : v)
    {
        se::Value out = se::Value::Null;
        ccvaluemap_to_seval(valueMap, &out);
        arr->setArrayElement(i, out);

        ++i;
    }

    return true;
}

#if USE_GFX_RENDERER
bool Rect_to_seval(const cocos2d::renderer::Rect &v, se::Value *ret)
{
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("x", se::Value(v.x));
    obj->setProperty("y", se::Value(v.y));
    obj->setProperty("w", se::Value(v.w));
    obj->setProperty("h", se::Value(v.h));
    ret->setObject(obj);

    return true;
}

bool EffectProperty_to_seval(const cocos2d::renderer::Effect::Property &v, se::Value *ret)
{
    assert(ret != nullptr);

    auto type = v.getType();
    if (type == cocos2d::renderer::Technique::Parameter::Type::TEXTURE_2D || type == cocos2d::renderer::Technique::Parameter::Type::TEXTURE_CUBE)
    {
        auto count = v.getCount();
        if (0 == count)
        {
            *ret = se::Value::Null;
        }
        else if (1 == count)
        {
            se::Value val;
            native_ptr_to_seval<cocos2d::renderer::Texture>(v.getTexture(), &val);
            *ret = val;
        }
        else
        {
            auto texArray = v.getTextureArray();
            se::HandleObject arr(se::Object::createArrayObject(count));
            for (uint8_t i = 0; i < count; ++i)
            {
                se::Value val;
                native_ptr_to_seval<cocos2d::renderer::Texture>(texArray[0], &val);
                arr->setArrayElement(i, val);
            }
            ret->setObject(arr);
        }
    }
    else
    {
        void *data = v.getValue();

        switch (type)
        {
        case cocos2d::renderer::Technique::Parameter::Type::INT:
            ret->setInt32(*((int32_t *)data));
            break;
        case cocos2d::renderer::Technique::Parameter::Type::INT2:
            toVec2(data, DataType::INT, ret);
            break;
        case cocos2d::renderer::Technique::Parameter::Type::INT3:
            toVec3(data, DataType::INT, ret);
            break;
        case cocos2d::renderer::Technique::Parameter::Type::INT4:
            toVec4(data, DataType::INT, ret);
            break;
        case cocos2d::renderer::Technique::Parameter::Type::FLOAT:
            ret->setFloat(*((float *)data));
            break;
        case cocos2d::renderer::Technique::Parameter::Type::FLOAT2:
            toVec2(data, DataType::FLOAT, ret);
            break;
        case cocos2d::renderer::Technique::Parameter::Type::FLOAT3:
            toVec3(data, DataType::FLOAT, ret);
        case cocos2d::renderer::Technique::Parameter::Type::FLOAT4:
            toVec4(data, DataType::FLOAT, ret);
            break;
        case cocos2d::renderer::Technique::Parameter::Type::MAT2:
            toMat((float *)data, 4, ret);
            break;
        case cocos2d::renderer::Technique::Parameter::Type::MAT3:
            toMat((float *)data, 9, ret);
            break;
        case cocos2d::renderer::Technique::Parameter::Type::MAT4:
            toMat((float *)data, 16, ret);
            break;
        default:
            assert(false);
            break;
        }
    }

    return true;
}

bool std_unorderedmap_string_EffectProperty_to_seval(const std::unordered_map<std::string, cocos2d::renderer::Effect::Property> &v, se::Value *ret)
{
    assert(ret != nullptr);

    se::HandleObject obj(se::Object::createPlainObject());
    bool ok = true;
    for (const auto &e : v)
    {
        const std::string &key = e.first;
        const cocos2d::renderer::Effect::Property &value = e.second;

        if (key.empty())
            continue;

        se::Value tmp;
        if (!EffectProperty_to_seval(value, &tmp))
        {
            ok = false;
            ret->setUndefined();
            break;
        }

        obj->setProperty(key.c_str(), tmp);
    }

    if (ok)
        ret->setObject(obj);

    return ok;
}

bool VertexFormat_to_seval(const cocos2d::renderer::VertexFormat &v, se::Value *ret)
{
    assert(false);
    return true;
}

bool TechniqueParameter_to_seval(const cocos2d::renderer::Technique::Parameter &v, se::Value *ret)
{
    assert(ret != nullptr);

    se::Object *param = se::Object::createPlainObject();
    se::Value typeVal;
    auto type = v.getType();
    int32_to_seval((int32_t)type, &typeVal);
    param->setProperty("type", typeVal);

    se::Value nameVal;
    std_string_to_seval(v.getName(), &nameVal);
    param->setProperty("name", nameVal);

    ret->setObject(param);
    return true;
}

bool std_vector_TechniqueParameter_to_seval(const std::vector<cocos2d::renderer::Technique::Parameter> &v, se::Value *ret)
{
    assert(ret != nullptr);
    se::HandleObject arr(se::Object::createArrayObject(v.size()));
    ret->setObject(arr);

    uint32_t i = 0;
    for (const auto &param : v)
    {
        se::Value out = se::Value::Null;
        TechniqueParameter_to_seval(param, &out);
        arr->setArrayElement(i, out);
        ++i;
    }
    return true;
}

bool std_vector_RenderTarget_to_seval(const std::vector<cocos2d::renderer::RenderTarget *> &v, se::Value *ret)
{
    assert(false);
    return true;
}
#endif // USE_GFX_RENDERER > 0

#if USE_SPINE

bool seval_to_spine_Vector_String(const se::Value &v, spine::Vector<spine::String> *ret)
{
    assert(ret != nullptr);
    assert(v.isObject());
    se::Object *obj = v.toObject();
    assert(obj->isArray());

    bool ok = true;
    uint32_t len = 0;
    ok = obj->getArrayLength(&len);
    if (!ok)
    {
        ret->clear();
        return false;
    }

    se::Value tmp;
    for (uint32_t i = 0; i < len; ++i)
    {
        ok = obj->getArrayElement(i, &tmp);
        if (!ok || !tmp.isObject())
        {
            ret->clear();
            return false;
        }

        const char *str = tmp.toString().c_str();
        ret->add(str);
    }

    return true;
}

bool spine_Vector_String_to_seval(const spine::Vector<spine::String> &v, se::Value *ret)
{
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createArrayObject(v.size()));
    bool ok = true;

    spine::Vector<spine::String> tmpv = v;
    for (uint32_t i = 0, count = (uint32_t)tmpv.size(); i < count; i++)
    {
        if (!obj->setArrayElement(i, se::Value(tmpv[i].buffer())))
        {
            ok = false;
            ret->setUndefined();
            break;
        }
    }

    if (ok)
        ret->setObject(obj);

    return ok;
}
#endif
