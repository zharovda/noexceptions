//============================================================================================================
//	traits and type traits
//============================================================================================================
#ifndef __TRAITS_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_
#define __TRAITS_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_

namespace ccl
{
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

}

#endif // !__TRAITS_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_



