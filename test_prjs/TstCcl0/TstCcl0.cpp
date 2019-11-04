/*++

Module Name:

    

Abstract:

    

Environment:

    Kernel mode

--*/

#include <wdm.h>
#include <dontuse.h>


#include "tst_memoryh.h"


/*************************************************************************
    Prototypes
*************************************************************************/

extern "C" {

    DRIVER_INITIALIZE DriverEntry;
    NTSTATUS
        DriverEntry(
            _In_ PDRIVER_OBJECT DriverObject,
            _In_ PUNICODE_STRING RegistryPath
        );

    _Function_class_(DRIVER_UNLOAD)
    VOID
        TstDriverUnload(
            _In_ struct _DRIVER_OBJECT* DriverObject
        );
} // extern C


//
//  Assign text sections for each routine.
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#endif

/*************************************************************************
     initialization and unload routines.
*************************************************************************/

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING 
)
/*++

Routine Description:

    This is the initialization routine for this miniFilter driver.  This
    registers with FltMgr and initializes all global data structures.

Arguments:

    DriverObject - Pointer to driver object created by the system to
        represent this driver.

    RegistryPath - Unicode string identifying where the parameters for this
        driver are located in the registry.

Return Value:

    Routine can return non success error codes.

--*/
{
    constexpr NTSTATUS status = STATUS_SUCCESS;

    //driver callbacks initialization 
    DriverObject->DriverUnload = TstDriverUnload;

    test_memory_h();

    return status;
}

_Function_class_(DRIVER_UNLOAD)
VOID
TstDriverUnload(
    _In_ struct _DRIVER_OBJECT* 
)
{

}