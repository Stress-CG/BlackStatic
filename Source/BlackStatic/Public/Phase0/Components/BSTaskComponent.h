#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Phase0/BlackStaticPhase0Types.h"
#include "BSTaskComponent.generated.h"

class ABSWaterPowerObjectiveActor;
class UBSTaskDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBSTaskStateChangedSignature);

UCLASS(ClassGroup = (BlackStatic), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class BLACKSTATIC_API UBSTaskComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBSTaskComponent();

	UPROPERTY(BlueprintAssignable, Category = "Task")
	FBSTaskStateChangedSignature OnTaskStateChanged;

	UFUNCTION(BlueprintCallable, Category = "Task")
	bool AcceptTask(UBSTaskDefinition* InTaskDefinition, ABSWaterPowerObjectiveActor* InObjectiveActor, int32 InExpectedPartySize);

	UFUNCTION(BlueprintCallable, Category = "Task")
	void MarkSiteCompleted();

	UFUNCTION(BlueprintCallable, Category = "Task")
	void CompleteTask();

	UFUNCTION(BlueprintCallable, Category = "Task")
	void FailTask();

	UFUNCTION(BlueprintCallable, Category = "Task")
	void ClearTask();

	UFUNCTION(BlueprintPure, Category = "Task")
	bool HasActiveTask() const;

	UFUNCTION(BlueprintPure, Category = "Task")
	bool IsReadyForExtraction() const;

	UFUNCTION(BlueprintPure, Category = "Task")
	UBSTaskDefinition* GetActiveTask() const;

	UFUNCTION(BlueprintPure, Category = "Task")
	EBSObjectiveState GetObjectiveState() const;

	UFUNCTION(BlueprintPure, Category = "Task")
	int32 GetExpectedPartySize() const;

	UFUNCTION(BlueprintPure, Category = "Task")
	ABSWaterPowerObjectiveActor* GetObjectiveActor() const;

private:
	UPROPERTY(VisibleAnywhere, Category = "Task")
	EBSObjectiveState ObjectiveState = EBSObjectiveState::None;

	UPROPERTY(VisibleAnywhere, Category = "Task")
	TObjectPtr<UBSTaskDefinition> TaskDefinition;

	UPROPERTY(VisibleAnywhere, Category = "Task")
	TObjectPtr<ABSWaterPowerObjectiveActor> ObjectiveActor;

	UPROPERTY(VisibleAnywhere, Category = "Task")
	int32 ExpectedPartySize = 1;
};
