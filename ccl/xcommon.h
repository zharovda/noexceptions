//============================================================================================================
//	internal header (std xtr1common.h like)
//============================================================================================================
#ifndef __XCOMMON_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_
#define __XCOMMON_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_

namespace ccl
{

//------------------------------------------------------------------------------------------------------------
// templates copied as-is from std 
//------------------------------------------------------------------------------------------------------------
template <class _Ty>
struct remove_reference {
    using type = _Ty;
};

template <class _Ty>
using remove_reference_t = typename remove_reference<_Ty>::type;


// STRUCT TEMPLATE remove_extent
template <class _Ty>
struct remove_extent { // remove array extent
    using type = _Ty;
};
 
template <class _Ty>
using remove_extent_t = typename remove_extent<_Ty>::type;


} // namespace ccl

#endif // !__XCOMMON_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_