#pragma once

#include "common/TyObject.h"

namespace ty
{

//! List of native types to be exported as
enum class NativeType
{
    I_32,
    I_64
};

//! Creates a string representation for a NativeType (as expected by LLVM-IR)
inline auto const* to_string(NativeType const n)
{
    switch (n)
    {
    case NativeType::I_32:		return "i32";
    default:	
        std::abort(); 
        return "";
    }
}

//! Returns the default alignment for a native type
inline auto alignment_of(NativeType const n)
{
    switch (n)
    {
    case NativeType::I_32:		return 4;
    default:	
        std::abort(); 
        return 0;
    }
}

class Type : public TyObject<>
{
public:
    virtual bool operator==(Type const&) const = 0;

    virtual bool operator!=(Type const& t) const { return !(*this == t); }

    virtual ~Type() = default;
};

class SystemType : public Type
{
public:
    explicit SystemType(NativeType nt, int align)
        : m_native_type{ nt }, m_alignment{ align }
    {}

    bool operator==(Type const& other) const override {
        if (auto* o = dynamic_cast<SystemType const*>(&other))
        {
            return native_type() == o->native_type();
        }
        return false;
    }

    NativeType native_type() const { return m_native_type; }

    int alignment() const { return m_alignment; }

private:
    NativeType	m_native_type{ NativeType::I_32 };

    int			m_alignment{ 0 };
};

class NumericType : public SystemType
{
public:
    template <typename... Args>
    explicit NumericType(Args&&... args)
        : SystemType(std::forward<Args>(args)...) {}
};

class Int32Type : public NumericType
{
public:
    explicit Int32Type()
        : NumericType{ NativeType::I_32, alignment_of(NativeType::I_32) }
    {}
};

} // namespace ty
