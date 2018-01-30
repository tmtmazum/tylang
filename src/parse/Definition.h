#pragma once

namespace ty
{

enum class NativeType
{
	I_32,
	I_64
};

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

class Definition
{
public:
	virtual ~Definition() = default;
};

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

class Int32Definition : public DataDefinition
{
public:
	explicit Int32Definition(int value = 0)
		: DataDefinition{ NativeType::I_32, value, alignment_of(NativeType::I_32) }
	{}
};


} // namespace ty
