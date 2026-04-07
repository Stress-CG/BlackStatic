#include "Phase0/Actors/BSExtractionPoint.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Phase0/Actors/BSPhase0Character.h"
#include "Phase0/Components/BSTaskComponent.h"
#include "Phase0/Data/BSTaskDefinition.h"
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
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Silver, TEXT("No completed field task ready for extraction."));
		}
		return;
	}

	if (UBSSettlementSubsystem* SettlementSubsystem = GetGameInstance()->GetSubsystem<UBSSettlementSubsystem>())
	{
		SettlementSubsystem->ApplyTaskReward(ActiveTask);
	}

	TaskComponent->CompleteTask();
	TaskComponent->ClearTask();
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Extraction successful. Settlement reward applied."));
	}
}

FText ABSExtractionPoint::GetInteractionPrompt_Implementation() const
{
	return NSLOCTEXT("BlackStatic", "ExtractionPrompt", "Extract To Settlement");
}
