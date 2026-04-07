#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BSPhase0GameMode.generated.h"

class ABSPhase0Character;

UCLASS()
class BLACKSTATIC_API ABSPhase0GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABSPhase0GameMode();

	void HandleSurvivorDeath(ABSPhase0Character* DeadCharacter);

private:
	void RespawnController(TWeakObjectPtr<AController> ControllerToRespawn);

	UPROPERTY(EditDefaultsOnly, Category = "Phase0")
	float SurvivorRespawnDelaySeconds = 2.0f;

	FTimerHandle SurvivorRespawnTimerHandle;
};
