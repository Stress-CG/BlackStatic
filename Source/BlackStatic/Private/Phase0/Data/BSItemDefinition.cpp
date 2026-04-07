#include "Phase0/Data/BSItemDefinition.h"

FName UBSItemDefinition::ResolveItemId() const
{
	return !ItemId.IsNone() ? ItemId : GetFName();
}
