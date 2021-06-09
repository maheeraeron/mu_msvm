/*++

Copyright (c) Microsoft Corporation

Module Name:

    Paging

Abstract:

    Return Paging attribute.

--*/

#include "CpuPageTable.h"


/**
  Get paging details.

  @param  PagingContextData      The paging context.
  @param  PageTableBase          Return PageTableBase field.
  @param  Attributes             Return Attributes field.

**/
VOID
GetPagingDetails (
  IN  PAGE_TABLE_LIB_PAGING_CONTEXT_DATA *PagingContextData,
  OUT UINTN                              **PageTableBase     OPTIONAL,
  OUT UINT32                             **Attributes        OPTIONAL
  )
{
  if (PageTableBase != NULL) {
    *PageTableBase = &PagingContextData->Ia32.PageTableBase;
  }
  if (Attributes != NULL) {
    *Attributes = &PagingContextData->Ia32.Attributes;
  }
}

