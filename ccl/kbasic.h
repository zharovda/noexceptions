//============================================================================================================
//	Code wrappers uses to remove dependencies from WDM structures and macroes.
//============================================================================================================
#ifndef __KBASIC_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_
#define __KBASIC_H_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_


// common code library
namespace ccl	
{

// kernel basic wrappers
namespace kbasic 
{

// ntstatus.h values equivalents
enum class NtStatusVals : long	
{
	NtStatusSuccess =               ((long)0x00000000L),    // STATUS_SUCCESS
	NtStatusUnsuccessful =          ((long)0xC0000001L),    // STATUS_UNSUCCESSFUL
	NtStatusInvalidParameter =      ((long)0xC000000DL),    // STATUS_INVALID_PARAMETER
	NtStatusInsufficientResources = ((long)0xC000009AL)     // STATUS_INSUFFICIENT_RESOURCES
};


// native status code type definition using preprocessor 
using NtStatus = long;


// status value conversion to native error value
constexpr NtStatus status(const NtStatusVals& st) { return static_cast<NtStatus>(st); }


// status code check 
inline bool ntsuccess(const NtStatus& st) noexcept { return st >= 0; }


} // namespace kbasic 

} // namespace ccl

#endif // !_AF36F6FA_1D5A_4D82_8F95_EB1CA596679A_
