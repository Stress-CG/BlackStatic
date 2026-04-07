#include "Phase0/Data/BSTaskDefinition.h"

FName UBSTaskDefinition::ResolveTaskId() const
{
	return !TaskId.IsNone() ? TaskId : GetFName();
}
