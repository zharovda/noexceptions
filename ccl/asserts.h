//============================================================================================================
//	Assertion wrappers declaration.
//============================================================================================================
#ifndef __ASSERTS_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_
#define __ASSERTS_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_

namespace ccl
{

namespace asserts
{

#ifdef __KERNEL_MODE

//------------------------------------------------------------------------------------------------------------
//  assert macro definition for kernel mode 
//------------------------------------------------------------------------------------------------------------
#ifndef assert
#ifdef DBG
#define assert(expression) \
    ((!(exp)) ? \
        (RtlAssert( (PVOID)#expression, (PVOID)__FILE__, __LINE__, NULL ),FALSE) : \
        TRUE)
#else // DBG
#define assert(expression) ((void)0)
#endif // DBG
#endif // assert

#else // __KERNEL_MODE
//------------------------------------------------------------------------------------------------------------
//  for user mode include assert.h is enought
//------------------------------------------------------------------------------------------------------------
#include <assert.h>
#endif // __KERNEL_MODE

} // namespace asserts

} // namespace ccl

#endif // !__ASSERTS_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_

