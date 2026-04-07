#include "Phase0/Actors/BSStashActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Phase0/Actors/BSPhase0Character.h"
#include "Phase0/Components/BSInventoryComponent.h"
#include "Phase0/Systems/BSSettlementSubsystem.h"
#include "UObject/ConstructorHelpers.h"

ABSStashActor::ABSStashActor()
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
		MeshComponent->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.8f));
	}
}

bool ABSStashActor::CanInteract_Implementation(AActor* Interactor) const
{
	const ABSPhase0Character* Character = Cast<ABSPhase0Character>(Interactor);
	return Character && Character->GetInventoryComponent();
}

void ABSStashActor::Interact_Implementation(AActor* Interactor)
{
	ABSPhase0Character* Character = Cast<ABSPhase0Character>(Interactor);
	if (!Character || !Character->GetInventoryComponent() || !GetGameInstance())
	{
		return;
	}

	UBSSettlementSubsystem* SettlementSubsystem = GetGameInstance()->GetSubsystem<UBSSettlementSubsystem>();
	if (!SettlementSubsystem)
	{
		return;
	}

	if (Character->GetInventoryComponent()->GetItems().Num() > 0)
	{
		SettlementSubsystem->DepositInventory(Character->GetInventoryComponent());
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Green, TEXT("Deposited carried items into the persistent stash."));
		}
	}
	else if (GEngine)
	{
		const FBSSettlementState& SettlementState = SettlementSubsystem->GetSettlementState();
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.0f,
			FColor::Cyan,
			FString::Printf(TEXT("Stash ready. Reputation %.0f | USD %d | Power %d | Water %d"), SettlementState.Reputation, SettlementState.USD, SettlementState.Power, SettlementState.Water)
		);
	}
}

FText ABSStashActor::GetInteractionPrompt_Implementation() const
{
	return NSLOCTEXT("BlackStatic", "StashPrompt", "Deposit Carried Items");
}
