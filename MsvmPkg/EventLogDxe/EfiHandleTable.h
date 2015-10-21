#pragma once


#define EFI_INVALID_HANDLE (EFI_HANDLE)((UINTN)-1)

//
// Channel handles are divided into two parts, the lower bits
// are reserved for use by the table itself and the remaining bits
// are available for use by callers.
//
#define HANDLE_TABLE_RESERVED_MASK  0x00ffffff


/*++

Routine Description:

    Allocates memory for an object in a handle table or the table itself.

Arguments:

    Size    Number of bytes of memory to allocate.

Return Value:

    Pointer to allocated memory or NULL on error.

--*/
typedef
VOID *
(EFIAPI *HANDLE_MEMORY_ALLOCATE)(
    _In_    UINTN           Size
    );


/*++

Routine Description:

    Frees memory previously allocated with HANDLE_MEMORY_ALLOCATE

Arguments:

    None.

Return Value:

    None

--*/
typedef
VOID
(EFIAPI *HANDLE_MEMORY_FREE)(
    );


/*++

Routine Description:

    Handle table object enumeration callback.

Arguments:

    TableHandle         Handle for EFI handle table that the enumerated object belongs to.

    CallbackContext     Context provided to the call to EfiHandleTableEnumerateObjects

    ObjectHandle        Handle representing the object

    Object              Object itself

Return Value:

    EFI_STATUS.
    Enumeration can be stopped by returning a non-success status.

--*/
typedef
EFI_STATUS
(EFIAPI *HANDLE_ENUMERATE_CALLBACK)(
    _In_    const EFI_HANDLE                TableHandle,
    _In_    VOID                           *CallbackContext,
    _In_    EFI_HANDLE                      ObjectHandle,
    _In_    VOID                           *Object
    );


//
// Describes information about a handle table
//
typedef struct
{
    HANDLE_MEMORY_ALLOCATE  Allocate;
    HANDLE_MEMORY_FREE      Free;
    UINTN                   ObjectKeySize;
} EFI_HANDLE_TABLE_INFO;


EFI_STATUS
EfiHandleTableInitialize(
    _In_        const EFI_HANDLE_TABLE_INFO    *Attributes,
    _In_        const UINT32                Size,
    _In_        const UINT8                 TableKey,
    _Out_       EFI_HANDLE                 *Table
    );


EFI_STATUS
EfiHandleTableAllocateObject(
    _In_        EFI_HANDLE                  TableHandle,
    _In_        const UINTN                 ObjectSize,
    _Outptr_    VOID                      **Object,
    _Out_       EFI_HANDLE                 *Handle
    );


VOID *
EfiHandleTableLookupByKey(
    _In_        const EFI_HANDLE            TableHandle,
    _In_bytecount_(KeySize)
                const VOID                 *Key,
    _In_        const UINT32                KeySize,
    _Out_opt_   EFI_HANDLE                 *Handle
    );


VOID *
EfiHandleTableLookupByHandle(
    _In_        const EFI_HANDLE            TableHandle,
    _In_        const EFI_HANDLE            Handle
    );


EFI_STATUS
EfiHandleTableEnumerateObjects(
    _In_        const EFI_HANDLE            TableHandle,
    _In_        const VOID                 *CallbackContext,
    _In_        HANDLE_ENUMERATE_CALLBACK   Callback
    );