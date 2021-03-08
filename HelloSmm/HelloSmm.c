#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/MmServicesTableLib.h>
#include <IndustryStandard/Q35MchIch9.h>

STATIC EFI_MM_CPU_IO_PROTOCOL *mMmCpuIo;

EFI_STATUS
EFIAPI
SmiHandler (
    IN EFI_HANDLE DispatchHandle,
    IN CONST VOID* RegisterContext,
    IN OUT VOID* CommBuffer,
    IN OUT UINTN* CommBufferSize
    )
{
    EFI_STATUS status;
    UINT8 commandNumber;

    //
    // Read the SMI command value from the power management port. This port can
    // be different on the other platforms, but this works on my target and all
    // Intel systems I have. You may fetch the AX register value to check this
    // using gEfiMmCpuProtocolGuid.
    //
    status = mMmCpuIo->Io.Read(mMmCpuIo, MM_IO_UINT8, ICH9_APM_CNT, 1, &commandNumber);
    ASSERT_EFI_ERROR(status);

    //
    // For the demonstration purpose ignore 0xff, which is pretty busy SMI.
    //
    if (commandNumber == 0xff)
    {
        goto Exit;
    }

    DEBUG((EFI_D_INFO, "[HelloSmm] SMI 0x%02x\n", commandNumber));

Exit:
    //
    // Allow other SMI to run.
    //
    return EFI_WARN_INTERRUPT_SOURCE_QUIESCED;
}

EFI_STATUS
EFIAPI
HelloSmmInitialize (
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE* SystemTable
    )
{
    EFI_STATUS status;
    EFI_HANDLE dispatchHandle;

    DEBUG((EFI_D_INFO, "[HelloSmm] HelloSmmInitialize called\n"));

    status = gMmst->MmLocateProtocol(&gEfiMmCpuIoProtocolGuid, NULL, (VOID **)&mMmCpuIo);
    ASSERT_EFI_ERROR(status);

    //
    // Register the root SMI handler.
    //
    status = gMmst->MmiHandlerRegister(SmiHandler, NULL, &dispatchHandle);
    ASSERT_EFI_ERROR(status);

    return status;
}
