#pragma once

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

//! Base class for a symbol definition
class Definition
{
public:
	virtual ~Definition() = default;
};

//! Definition of a data variable
class DataDefinition : public Definition
{
public:
	DataDefinition(NativeType nt, int val, int align)
		: m_native_type{ nt }, m_value{ val }, m_alignment{ align }
	{}

	NativeType native_type() const { return m_native_type; }

	int value() const { return m_value; }

	int alignment() const { return m_alignment; }

private:
	NativeType	m_native_type{ NativeType::I_32 };

	int			m_value{ 0 };

	int			m_alignment{ 0 };
};

//! Specialization of a definition of the int-32 type
class Int32Definition : public DataDefinition
{
public:
	explicit Int32Definition(int value = 0)
		: DataDefinition{ NativeType::I_32, value, alignment_of(NativeType::I_32) }
	{}
};


} // namespace ty
