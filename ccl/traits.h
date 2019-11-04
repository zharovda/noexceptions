//============================================================================================================
//	traits and type traits
//============================================================================================================
#ifndef __TRAITS_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_
#define __TRAITS_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_

#include ".\xcommon.h"

namespace ccl
{

// STRUCT TEMPLATE is_lvalue_reference
template <class>
constexpr bool is_lvalue_reference_v = false; // determine whether type argument is an lvalue reference

template <class _Ty>
constexpr bool is_lvalue_reference_v<_Ty&> = true;

template <class _Ty>
struct is_lvalue_reference : bool_constant<is_lvalue_reference_v<_Ty>> {};


// STRUCT TEMPLATE _Add_reference
template <class _Ty, class = void>
struct _Add_reference { // add reference (non-referenceable type)
    using _Lvalue = _Ty;
    using _Rvalue = _Ty;
};

template <class _Ty>
using add_lvalue_reference_t = typename _Add_reference<_Ty>::_Lvalue;

template <class _Ty>
constexpr bool is_nothrow_move_constructible_v = __is_nothrow_constructible(_Ty, _Ty);

template <class _Ty>
constexpr bool is_nothrow_move_assignable_v = __is_nothrow_assignable(add_lvalue_reference_t<_Ty>, _Ty);






// FUNCTION TEMPLATE forward
template <class _Ty>
 constexpr _Ty&& forward(
    remove_reference_t<_Ty>& _Arg) noexcept { // forward an lvalue as either an lvalue or an rvalue
    return static_cast<_Ty&&>(_Arg);
}

template <class _Ty>
constexpr _Ty&& forward(remove_reference_t<_Ty>&& _Arg) noexcept { // forward an rvalue as an rvalue
    static_assert(!is_lvalue_reference_v<_Ty>, "bad forward call");
    return static_cast<_Ty&&>(_Arg);
}



}

#endif // !__TRAITS_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_



