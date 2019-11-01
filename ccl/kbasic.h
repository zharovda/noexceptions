//============================================================================================================
//	Code wrappers uses to remove dependencies from WDM structures and macroes.
//============================================================================================================
#ifndef __KBASIC_H__
#define __KBASIC_H__


// common code library
namespace ccl	
{

// kernel basic wrappers
namespace kbasic 
{

// ntstatus.h values equivalents
enum class NtStatusVals : long	
{
	NtStatusSuccess =               0x00000000L,    // STATUS_SUCCESS
	NtStatusUnsuccessful =          0xC0000001L,    // STATUS_UNSUCCESSFUL
	NtStatusInvalidParameter =      0xC000000DL,    // STATUS_INVALID_PARAMETER
	NtStatusInsufficientResources = 0xC000009AL     // STATUS_INSUFFICIENT_RESOURCES
};


// native status code type definition using preprocessor 
#ifndef __KERNEL_MODE
using NtStatus = long;
#else	// __KERNEL_MODE
using NtStatus = NTSTATUS;
#endif  // __KERNEL_MODE


// status value conversion to native error value
constexpr NtStatus status(NtStatusVals st) { return static_cast<NtStatus>(st); }


// status code check 
inline bool ntsuccess(const NtStatus& st) { return st >= 0; }


} // namespace kbasic 

} // namespace ccl

#endif // !__KBASIC_H__
