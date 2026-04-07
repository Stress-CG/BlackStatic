#include "Phase0/Actors/BSObjectivePickup.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Phase0/Actors/BSPhase0Character.h"
#include "Phase0/Components/BSInventoryComponent.h"
#include "Phase0/Components/BSNoiseEmitterComponent.h"
#include "Phase0/Data/BSItemDefinition.h"
#include "Phase0/Framework/BSPhase0PlayerController.h"
#include "UObject/ConstructorHelpers.h"

ABSObjectivePickup::ABSObjectivePickup()
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
		MeshComponent->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.12f));
	}
}

bool ABSObjectivePickup::CanInteract_Implementation(AActor* Interactor) const
{
	const ABSPhase0Character* Character = Cast<ABSPhase0Character>(Interactor);
	return Character && Character->GetInventoryComponent() && ItemDefinition;
}

void ABSObjectivePickup::Interact_Implementation(AActor* Interactor)
{
	ABSPhase0Character* Character = Cast<ABSPhase0Character>(Interactor);
	if (!Character || !Character->GetInventoryComponent() || !ItemDefinition)
	{
		return;
	}

	Character->GetInventoryComponent()->AddItemDefinition(ItemDefinition, Quantity);
	if (UBSNoiseEmitterComponent* Emitter = Character->FindComponentByClass<UBSNoiseEmitterComponent>())
	{
		Emitter->EmitNoise(EBSNoiseType::Tool, PickupNoiseLoudness, TEXT("Pickup"), GetActorLocation());
	}

	if (ABSPhase0PlayerController* Phase0Controller = Character->GetController<ABSPhase0PlayerController>())
	{
		const FString ItemName = ItemDefinition->DisplayName.IsEmpty() ? ItemDefinition->GetName() : ItemDefinition->DisplayName.ToString();
		Phase0Controller->ShowPhase0Message(
			FText::FromString(FString::Printf(TEXT("Recovered %s x%d. Check your backpack and keep moving."), *ItemName, Quantity)),
			FLinearColor(0.92f, 0.86f, 0.28f),
			3.5f);
	}

	Destroy();
}

FText ABSObjectivePickup::GetInteractionPrompt_Implementation() const
{
	if (ItemDefinition && !ItemDefinition->DisplayName.IsEmpty())
	{
		return FText::Format(NSLOCTEXT("BlackStatic", "PickupPrompt", "Pick Up {0}"), ItemDefinition->DisplayName);
	}

	return NSLOCTEXT("BlackStatic", "PickupFallbackPrompt", "Pick Up Objective Item");
}
