#include "Phase0/Actors/BSExtractionPoint.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Phase0/Actors/BSPhase0Character.h"
#include "Phase0/Components/BSTaskComponent.h"
#include "Phase0/Data/BSTaskDefinition.h"
#include "Phase0/Framework/BSPhase0PlayerController.h"
#include "Phase0/Systems/BSSettlementSubsystem.h"
#include "UObject/ConstructorHelpers.h"

ABSExtractionPoint::ABSExtractionPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (MeshFinder.Succeeded())
	{
		MeshComponent->SetStaticMesh(MeshFinder.Object);
		MeshComponent->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.08f));
	}
}

bool ABSExtractionPoint::CanInteract_Implementation(AActor* Interactor) const
{
	const ABSPhase0Character* Character = Cast<ABSPhase0Character>(Interactor);
	return Character && Character->GetTaskComponent();
}

void ABSExtractionPoint::Interact_Implementation(AActor* Interactor)
{
	ABSPhase0Character* Character = Cast<ABSPhase0Character>(Interactor);
	if (!Character || !Character->GetTaskComponent() || !GetGameInstance())
	{
		return;
	}

	UBSTaskComponent* TaskComponent = Character->GetTaskComponent();
	UBSTaskDefinition* ActiveTask = TaskComponent->GetActiveTask();
	if (!ActiveTask || !TaskComponent->IsReadyForExtraction())
	{
		if (ABSPhase0PlayerController* Phase0Controller = Character->GetController<ABSPhase0PlayerController>())
		{
			Phase0Controller->ShowPhase0Message(
				NSLOCTEXT("BlackStatic", "ExtractNotReady", "No completed field task is ready to extract yet. Finish the site work first."),
				FLinearColor(0.80f, 0.82f, 0.86f),
				3.5f);
		}
		return;
	}

	if (UBSSettlementSubsystem* SettlementSubsystem = GetGameInstance()->GetSubsystem<UBSSettlementSubsystem>())
	{
		SettlementSubsystem->ApplyTaskReward(ActiveTask);
	}

	TaskComponent->CompleteTask();
	TaskComponent->ClearTask();
	if (ABSPhase0PlayerController* Phase0Controller = Character->GetController<ABSPhase0PlayerController>())
	{
		Phase0Controller->ShowPhase0Message(
			NSLOCTEXT("BlackStatic", "ExtractSuccess", "Extraction successful. Settlement reward applied and progression advanced."),
			FLinearColor(0.22f, 0.78f, 0.46f),
			4.0f);
	}
}

FText ABSExtractionPoint::GetInteractionPrompt_Implementation() const
{
	return NSLOCTEXT("BlackStatic", "ExtractionPrompt", "Extract To Settlement");
}
