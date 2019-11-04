//============================================================================================================
//	common code library utility declaration 
//============================================================================================================
#ifndef __UTILITY_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_
#define __UTILITY_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_

#include ".\traits.h"
#include ".\xcommon.h"

namespace ccl
{

//------------------------------------------------------------------------------------------------------------
// templates copied as-is from std (_NODISCARD - removed)
//------------------------------------------------------------------------------------------------------------

#pragma warning(disable : 26487)
template <class _Ty>
_NODISCARD constexpr remove_reference_t<_Ty>&& move(_Ty&& _Arg) noexcept { // forward _Arg as movable
    return static_cast<remove_reference_t<_Ty>&&>(_Arg);
}
#pragma warning(default : 26487)

template <class _Ty, class>
void swap(_Ty& _Left, _Ty& _Right) noexcept 
{ // exchange values stored at _Left and _Right
    _Ty _Tmp =  ccl::move(_Left);
    _Left = ccl::move(_Right);
    _Right = ccl::move(_Tmp);
}


} // namespace ccl

#endif // !__UTILITY_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_