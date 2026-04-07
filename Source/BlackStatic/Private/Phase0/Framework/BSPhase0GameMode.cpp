#include "Phase0/Framework/BSPhase0GameMode.h"

#include "Engine/Engine.h"
#include "GameFramework/Controller.h"
#include "Phase0/Actors/BSPhase0Character.h"
#include "Phase0/Components/BSTaskComponent.h"
#include "Phase0/Framework/BSPhase0PlayerController.h"

ABSPhase0GameMode::ABSPhase0GameMode()
{
	DefaultPawnClass = ABSPhase0Character::StaticClass();
	PlayerControllerClass = ABSPhase0PlayerController::StaticClass();
}

void ABSPhase0GameMode::HandleSurvivorDeath(ABSPhase0Character* DeadCharacter)
{
	if (!DeadCharacter)
	{
		return;
	}

	if (UBSTaskComponent* TaskComponent = DeadCharacter->GetTaskComponent())
	{
		TaskComponent->FailTask();
	}

	AController* Controller = DeadCharacter->GetController();
	if (!Controller)
	{
		DeadCharacter->SetLifeSpan(4.0f);
		return;
	}

	DeadCharacter->DetachFromControllerPendingDestroy();
	DeadCharacter->SetLifeSpan(4.0f);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("Survivor lost. Stash retained. Spawning a new survivor."));
	}

	if (ABSPhase0PlayerController* Phase0Controller = Cast<ABSPhase0PlayerController>(Controller))
	{
		Phase0Controller->ShowPhase0Message(
			NSLOCTEXT("BlackStatic", "DeathLoopMessage", "Survivor lost. Backpack gone. Stash retained. A new survivor is inbound."),
			FLinearColor(0.85f, 0.25f, 0.25f),
			5.0f);
	}

	FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(this, &ABSPhase0GameMode::RespawnController, TWeakObjectPtr<AController>(Controller));
	GetWorldTimerManager().SetTimer(SurvivorRespawnTimerHandle, RespawnDelegate, SurvivorRespawnDelaySeconds, false);
}

void ABSPhase0GameMode::RespawnController(TWeakObjectPtr<AController> ControllerToRespawn)
{
	if (!ControllerToRespawn.IsValid())
	{
		return;
	}

	RestartPlayer(ControllerToRespawn.Get());
}
