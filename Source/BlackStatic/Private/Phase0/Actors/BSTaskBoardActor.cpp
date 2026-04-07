#include "Phase0/Actors/BSTaskBoardActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Phase0/Actors/BSPhase0Character.h"
#include "Phase0/Actors/BSWaterPowerObjectiveActor.h"
#include "Phase0/Components/BSTaskComponent.h"
#include "Phase0/Components/BSSurvivorStateComponent.h"
#include "Phase0/Data/BSTaskDefinition.h"
#include "UObject/ConstructorHelpers.h"

ABSTaskBoardActor::ABSTaskBoardActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (MeshFinder.Succeeded())
	{
		MeshComponent->SetStaticMesh(MeshFinder.Object);
		MeshComponent->SetRelativeScale3D(FVector(0.4f, 0.1f, 0.6f));
	}
}

bool ABSTaskBoardActor::CanInteract_Implementation(AActor* Interactor) const
{
	const ABSPhase0Character* Character = Cast<ABSPhase0Character>(Interactor);
	return Character && Character->GetTaskComponent() && TaskDefinition && LinkedObjective;
}

void ABSTaskBoardActor::Interact_Implementation(AActor* Interactor)
{
	ABSPhase0Character* Character = Cast<ABSPhase0Character>(Interactor);
	if (!Character || !Character->GetTaskComponent() || !TaskDefinition || !LinkedObjective)
	{
		return;
	}

	UBSTaskComponent* TaskComponent = Character->GetTaskComponent();
	if (TaskComponent->HasActiveTask())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Silver, TEXT("Task already active. Complete or fail your current run first."));
		}
		return;
	}

	const int32 PartySize = EstimatePartySize();
	if (TaskComponent->AcceptTask(TaskDefinition, LinkedObjective, PartySize))
	{
		LinkedObjective->ActivateObjective(PartySize);
		if (GEngine)
		{
			const FString TaskName = TaskDefinition->DisplayName.IsEmpty() ? TaskDefinition->GetName() : TaskDefinition->DisplayName.ToString();
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, FString::Printf(TEXT("Accepted task: %s | Party Size: %d"), *TaskName, PartySize));
		}
	}
}

FText ABSTaskBoardActor::GetInteractionPrompt_Implementation() const
{
	return NSLOCTEXT("BlackStatic", "TaskBoardPrompt", "Accept Water & Power Restoration Task");
}

int32 ABSTaskBoardActor::EstimatePartySize() const
{
	if (!GetWorld())
	{
		return 1;
	}

	int32 PartySize = 0;
	for (TActorIterator<ABSPhase0Character> It(GetWorld()); It; ++It)
	{
		const ABSPhase0Character* Candidate = *It;
		if (!Candidate || !Candidate->GetSurvivorStateComponent() || !Candidate->GetSurvivorStateComponent()->IsAlive())
		{
			continue;
		}

		if (FVector::Distance(Candidate->GetActorLocation(), GetActorLocation()) <= PartyAssemblyRadius)
		{
			++PartySize;
		}
	}

	return FMath::Max(1, PartySize);
}
