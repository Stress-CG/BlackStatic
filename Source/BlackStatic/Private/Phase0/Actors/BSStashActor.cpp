#include "Phase0/Actors/BSStashActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Phase0/Actors/BSPhase0Character.h"
#include "Phase0/Components/BSInventoryComponent.h"
#include "Phase0/Framework/BSPhase0PlayerController.h"
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
		if (ABSPhase0PlayerController* Phase0Controller = Character->GetController<ABSPhase0PlayerController>())
		{
			Phase0Controller->ShowPhase0Message(
				NSLOCTEXT("BlackStatic", "StashDeposited", "Deposited carried items into the persistent stash. Future survivors can reclaim them."),
				FLinearColor(0.22f, 0.78f, 0.46f),
				4.0f);
		}
	}
	else if (SettlementSubsystem->WithdrawAllStashToInventory(Character->GetInventoryComponent()))
	{
		if (ABSPhase0PlayerController* Phase0Controller = Character->GetController<ABSPhase0PlayerController>())
		{
			Phase0Controller->ShowPhase0Message(
				NSLOCTEXT("BlackStatic", "StashWithdrawn", "Reclaimed the persistent stash into your backpack. Carried gear is back in the field and at risk again."),
				FLinearColor(0.44f, 0.78f, 0.96f),
				4.5f);
		}
	}
	else if (ABSPhase0PlayerController* Phase0Controller = Character->GetController<ABSPhase0PlayerController>())
	{
		const FBSSettlementState& SettlementState = SettlementSubsystem->GetSettlementState();
		Phase0Controller->ShowPhase0Message(
			FText::FromString(FString::Printf(TEXT("Stash ready. Reputation %.0f | USD %d | Power %d | Water %d"), SettlementState.Reputation, SettlementState.USD, SettlementState.Power, SettlementState.Water)),
			FLinearColor(0.48f, 0.86f, 0.92f),
			4.0f);
	}
}

FText ABSStashActor::GetInteractionPrompt_Implementation() const
{
	return NSLOCTEXT("BlackStatic", "StashPrompt", "Deposit Or Reclaim Supplies");
}
