#include "UefiHavoc.h"

BOOLEAN
StringToValue(
    const STRING_VALUE_MAP  Map[],
    const CHAR16           *String,
    UINT32                 *Value
    )
{
    UINT32 i;
    BOOLEAN found = FALSE;

    i = 0;
    while (Map[i].String != NULL)
    {
        if (StrCmp(String, Map[i].String) == 0)
        {
            *Value = Map[i].Value;
            found = TRUE;
            break;
        }

        i++;
    }

    return found;
}


CHAR16*
ValueToString(
    const STRING_VALUE_MAP  Map[],
    UINT32                  Value
    )
{
    UINT32 i;

    i = 0;
    while (Map[i].String != NULL)
    {
        if (Map[i].Value == Value)
        {
            return Map[i].String;
        }

        i++;
    }

    return NULL;
}
