#include "Phase0/Actors/BSWaterPowerObjectiveActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "NavigationSystem.h"
#include "Phase0/Actors/BSInfectedCharacter.h"
#include "Phase0/Actors/BSPhase0Character.h"
#include "Phase0/Components/BSInventoryComponent.h"
#include "Phase0/Components/BSNoiseEmitterComponent.h"
#include "Phase0/Components/BSTaskComponent.h"
#include "Phase0/Data/BSTaskDefinition.h"
#include "Phase0/BlackStaticPhase0Statics.h"
#include "UObject/ConstructorHelpers.h"

ABSWaterPowerObjectiveActor::ABSWaterPowerObjectiveActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(SceneRoot);

	NoiseEmitterComponent = CreateDefaultSubobject<UBSNoiseEmitterComponent>(TEXT("NoiseEmitterComponent"));
	NoiseEmitterComponent->EmittedFaction = EBSFaction::Neutral;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (MeshFinder.Succeeded())
	{
		MeshComponent->SetStaticMesh(MeshFinder.Object);
		MeshComponent->SetRelativeScale3D(FVector(0.45f, 0.45f, 0.3f));
	}
}

bool ABSWaterPowerObjectiveActor::CanInteract_Implementation(AActor* Interactor) const
{
	const ABSPhase0Character* Character = Cast<ABSPhase0Character>(Interactor);
	if (!Character || !Character->GetTaskComponent() || !Character->GetInventoryComponent() || !TaskDefinition)
	{
		return false;
	}

	return Character->GetTaskComponent()->GetActiveTask() == TaskDefinition && !bSiteCompleted;
}

void ABSWaterPowerObjectiveActor::Interact_Implementation(AActor* Interactor)
{
	ABSPhase0Character* Character = Cast<ABSPhase0Character>(Interactor);
	if (!Character || !TaskDefinition)
	{
		return;
	}

	UBSTaskComponent* TaskComponent = Character->GetTaskComponent();
	UBSInventoryComponent* InventoryComponent = Character->GetInventoryComponent();
	if (!TaskComponent || !InventoryComponent || TaskComponent->GetActiveTask() != TaskDefinition)
	{
		return;
	}

	if (!InventoryComponent->HasRequiredItems(TaskDefinition->RequiredItems))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Orange, TEXT("Missing restoration parts. Find the battery and filter before installing."));
		}
		return;
	}

	InventoryComponent->ConsumeRequiredItems(TaskDefinition->RequiredItems);
	TaskComponent->MarkSiteCompleted();
	bSiteCompleted = true;
	if (NoiseEmitterComponent)
	{
		NoiseEmitterComponent->EmitNoise(EBSNoiseType::Objective, 1.1f, TEXT("RestoreUtilities"));
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Water and power restored. Return to settlement to extract the mission."));
	}
}

FText ABSWaterPowerObjectiveActor::GetInteractionPrompt_Implementation() const
{
	return NSLOCTEXT("BlackStatic", "ObjectivePrompt", "Install Battery And Filter");
}

void ABSWaterPowerObjectiveActor::ActivateObjective(const int32 PartySize)
{
	bObjectiveActive = true;
	bSiteCompleted = false;
	SpawnThreatWave(PartySize);
}

bool ABSWaterPowerObjectiveActor::IsObjectiveActive() const
{
	return bObjectiveActive;
}

void ABSWaterPowerObjectiveActor::SpawnThreatWave(const int32 PartySize)
{
	if (!GetWorld() || !InfectedClass || !TaskDefinition)
	{
		return;
	}

	const int32 DesiredCount = UBSPhase0Statics::ComputeScaledThreatCount(TaskDefinition->BaseThreatCount, PartySize);
	const int32 ExistingAliveCount = SpawnedThreats.FilterByPredicate([](const ABSInfectedCharacter* Infected)
	{
		return IsValid(Infected);
	}).Num();
	const int32 ToSpawn = FMath::Max(0, DesiredCount - ExistingAliveCount);
	if (ToSpawn == 0)
	{
		return;
	}

	if (UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		for (int32 Index = 0; Index < ToSpawn; ++Index)
		{
			FNavLocation SpawnLocation;
			if (!NavigationSystem->GetRandomReachablePointInRadius(GetActorLocation(), ObjectiveDefenseRadius, SpawnLocation))
			{
				continue;
			}

			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			ABSInfectedCharacter* Spawned = GetWorld()->SpawnActor<ABSInfectedCharacter>(InfectedClass, SpawnLocation.Location, FRotator::ZeroRotator, SpawnParameters);
			if (Spawned)
			{
				SpawnedThreats.Add(Spawned);
			}
		}
	}
}
