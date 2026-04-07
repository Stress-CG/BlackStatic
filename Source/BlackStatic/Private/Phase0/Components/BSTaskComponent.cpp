#include "Phase0/Components/BSTaskComponent.h"

UBSTaskComponent::UBSTaskComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UBSTaskComponent::AcceptTask(UBSTaskDefinition* InTaskDefinition, ABSWaterPowerObjectiveActor* InObjectiveActor, const int32 InExpectedPartySize)
{
	if (!InTaskDefinition || ObjectiveState == EBSObjectiveState::InProgress)
	{
		return false;
	}

	TaskDefinition = InTaskDefinition;
	ObjectiveActor = InObjectiveActor;
	ExpectedPartySize = FMath::Max(1, InExpectedPartySize);
	ObjectiveState = EBSObjectiveState::InProgress;
	OnTaskStateChanged.Broadcast();
	return true;
}

void UBSTaskComponent::MarkSiteCompleted()
{
	ObjectiveState = EBSObjectiveState::SiteCompleted;
	OnTaskStateChanged.Broadcast();
}

void UBSTaskComponent::CompleteTask()
{
	ObjectiveState = EBSObjectiveState::Completed;
	OnTaskStateChanged.Broadcast();
}

void UBSTaskComponent::FailTask()
{
	ObjectiveState = EBSObjectiveState::Failed;
	OnTaskStateChanged.Broadcast();
}

void UBSTaskComponent::ClearTask()
{
	ObjectiveState = EBSObjectiveState::None;
	TaskDefinition = nullptr;
	ObjectiveActor = nullptr;
	ExpectedPartySize = 1;
	OnTaskStateChanged.Broadcast();
}

bool UBSTaskComponent::HasActiveTask() const
{
	return ObjectiveState != EBSObjectiveState::None && ObjectiveState != EBSObjectiveState::Completed && ObjectiveState != EBSObjectiveState::Failed && TaskDefinition != nullptr;
}

bool UBSTaskComponent::IsReadyForExtraction() const
{
	return ObjectiveState == EBSObjectiveState::SiteCompleted || ObjectiveState == EBSObjectiveState::ExtractReady;
}

UBSTaskDefinition* UBSTaskComponent::GetActiveTask() const
{
	return TaskDefinition;
}

EBSObjectiveState UBSTaskComponent::GetObjectiveState() const
{
	return ObjectiveState;
}

int32 UBSTaskComponent::GetExpectedPartySize() const
{
	return ExpectedPartySize;
}

ABSWaterPowerObjectiveActor* UBSTaskComponent::GetObjectiveActor() const
{
	return ObjectiveActor;
}
