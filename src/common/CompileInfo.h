#pragma once

#include "TyObject.h"

namespace ty
{
    class CompileNotice
    {

    };

    enum class CompileWarning
    {

    };

    //! Fatal compile errors 
    enum class CompileError
    {
        unexpected_token,    // unexpected token in this context
        unexpected_end_of_file,
        deduced_type_mismatch,
        unsupported_feature,
        undefined_function_being_called,
        cannot_generate_undefined_symbol
    };
}; // namespace ty

#define NOT_YET_IMPLEMENTED() \
    std::exception((std::string("not yet implemented @ (") + __FUNCTION__ + ":" + std::to_string(__LINE__) + ")").c_str())
