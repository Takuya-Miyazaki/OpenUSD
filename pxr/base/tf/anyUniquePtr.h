//
// Copyright 2019 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_BASE_TF_ANY_UNIQUE_PTR_H
#define PXR_BASE_TF_ANY_UNIQUE_PTR_H

#include "pxr/pxr.h"
#include "pxr/base/tf/api.h"

#include <type_traits>

PXR_NAMESPACE_OPEN_SCOPE

/// A simple type-erased container that provides only destruction, moves and
/// immutable, untyped access to the held value.
///
/// There are a couple of facilities that provide fallback or default values
/// in error cases.  TfAnyUniquePtr exists to hold these oft-instantiated but
/// rarely accessed values.  As such, its design prioritizes compile-time
/// overhead over runtime performance and avoids clever metaprogramming.
/// Please resist the urge to add functionality to this class (e.g. small
/// object optimization, pxr_boost::python interoperability.)
class TfAnyUniquePtr
{
public:
    template <typename T>
    static TfAnyUniquePtr New() {
        static_assert(!std::is_array<T>::value, "Array types not supported");
        return TfAnyUniquePtr(new T());
    }

    template <typename T>
    static TfAnyUniquePtr New(T const &v) {
        static_assert(!std::is_array<T>::value, "Array types not supported");
        return TfAnyUniquePtr(new T(v));
    }

    TfAnyUniquePtr(TfAnyUniquePtr &&other)
        : _ptr(other._ptr)
        , _delete(other._delete)
    {
        other._ptr = nullptr;
        // We don't set other._delete to nullptr here on purpose.  Invoking
        // delete on a null pointer is not an error so if we can ensure that
        // _delete is never null we can call it unconditionally.
    }

    TfAnyUniquePtr& operator=(TfAnyUniquePtr &&other) {
        if (this != &other) {
            _delete(_ptr);
            _ptr = other._ptr;
            _delete = other._delete;
            other._ptr = nullptr;
        }
        return *this;
    }

    TfAnyUniquePtr(TfAnyUniquePtr const&) = delete;
    TfAnyUniquePtr& operator=(TfAnyUniquePtr const&) = delete;

    ~TfAnyUniquePtr() {
        _delete(_ptr);
    }

    /// Return a pointer to the owned object.
    void const *Get() const {
        return _ptr;
    }

private:
    template <typename T>
    explicit TfAnyUniquePtr(T const *ptr)
        : _ptr(ptr)
        , _delete(&_Delete<T>)
    {}

    template <typename T>
    static void _Delete(void const *ptr) {
        delete static_cast<T const *>(ptr);
    }

private:
    void const *_ptr;
    void (*_delete)(void const *);
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif
