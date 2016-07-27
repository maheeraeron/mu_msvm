
#pragma once

typedef struct
{
    CHAR16*     String;
    UINT32      Value;
}STRING_VALUE_MAP;


BOOLEAN
StringToValue(
    const STRING_VALUE_MAP  Map[],
    const CHAR16           *String,
    UINT32                 *Value
    );


CHAR16*
ValueToString(
    const STRING_VALUE_MAP  Map[],
    UINT32                  Value
    );
