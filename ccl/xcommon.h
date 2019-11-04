//============================================================================================================
//	internal header (std xtr1common.h like)
//============================================================================================================
#ifndef __XCOMMON_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_
#define __XCOMMON_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_

#if _HAS_NODISCARD
#define _NODISCARD [[nodiscard]]
#else // ^^^ CAN HAZ [[nodiscard]] / NO CAN HAZ [[nodiscard]] vvv
#define _NODISCARD
#endif // _HAS_NODISCARD


namespace ccl
{

//------------------------------------------------------------------------------------------------------------
// templates copied as-is from std 
//------------------------------------------------------------------------------------------------------------

    typedef decltype(nullptr) nullptr_t;


// STRUCT TEMPLATE remove_extent
    template <class _Ty>
    struct remove_extent { // remove array extent
        using type = _Ty;
    };

    template <class _Ty, size_t _Ix>
    struct remove_extent<_Ty[_Ix]> {
        using type = _Ty;
    };

    template <class _Ty>
    struct remove_extent<_Ty[]> {
        using type = _Ty;
    };

    template <class _Ty>
    using remove_extent_t = typename remove_extent<_Ty>::type;

// STRUCT TEMPLATE remove_reference
template <class _Ty>
struct remove_reference {
    using type = _Ty;
};

template <class _Ty>
struct remove_reference<_Ty&> {
    using type = _Ty;
};

template <class _Ty>
struct remove_reference<_Ty&&> {
    using type = _Ty;
};

template <class _Ty>
using remove_reference_t = typename remove_reference<_Ty>::type;




// STRUCT TEMPLATE integral_constant
template <class _Ty, _Ty _Val>
struct integral_constant {
    static constexpr _Ty value = _Val;

    using value_type = _Ty;
    using type = integral_constant;

    constexpr operator value_type() const noexcept {
        return value;
    }

    constexpr value_type operator()() const noexcept {
        return value;
    }
};

// ALIAS TEMPLATE bool_constant
template <bool _Val>
using bool_constant = integral_constant<bool, _Val>;

using true_type = bool_constant<true>;
using false_type = bool_constant<false>;

// TYPE PREDICATES
// STRUCT TEMPLATE is_array
template <class>
constexpr bool is_array_v = false; // determine whether type argument is an array

template <class _Ty, size_t _Nx>
constexpr bool is_array_v<_Ty[_Nx]> = true;

template <class _Ty>
constexpr bool is_array_v<_Ty[]> = true;

template <class _Ty>
struct is_array : bool_constant<is_array_v<_Ty>> {};

} // namespace ccl

#endif // !__XCOMMON_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_