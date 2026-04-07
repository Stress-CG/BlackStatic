#include "Phase0/Actors/BSInfectedCharacter.h"

#include "AIController.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Phase0/AI/BSInfectedAIController.h"
#include "Phase0/Actors/BSPhase0Character.h"
#include "Phase0/Components/BSNoiseEmitterComponent.h"
#include "Phase0/Components/BSNoiseListenerComponent.h"
#include "Phase0/Components/BSSurvivorStateComponent.h"

ABSInfectedCharacter::ABSInfectedCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	AIControllerClass = ABSInfectedAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	SurvivorStateComponent = CreateDefaultSubobject<UBSSurvivorStateComponent>(TEXT("SurvivorStateComponent"));
	NoiseListenerComponent = CreateDefaultSubobject<UBSNoiseListenerComponent>(TEXT("NoiseListenerComponent"));
	NoiseEmitterComponent = CreateDefaultSubobject<UBSNoiseEmitterComponent>(TEXT("NoiseEmitterComponent"));

	GetCharacterMovement()->MaxWalkSpeed = 250.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1500.0f;
}

void ABSInfectedCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (SurvivorStateComponent)
	{
		SurvivorStateComponent->OnDeath.AddDynamic(this, &ABSInfectedCharacter::HandleDeath);
	}

	if (NoiseListenerComponent)
	{
		NoiseListenerComponent->ListeningFaction = Faction;
	}

	if (NoiseEmitterComponent)
	{
		NoiseEmitterComponent->EmittedFaction = Faction;
	}
}

float ABSInfectedCharacter::TakeDamage(const float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	return SurvivorStateComponent ? SurvivorStateComponent->ApplyWorldDamage(DamageAmount) : Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

bool ABSInfectedCharacter::TryAttackTarget(AActor* TargetActor)
{
	if (!TargetActor || !GetWorld())
	{
		return false;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if ((CurrentTime - LastAttackTimeSeconds) < AttackCooldownSeconds)
	{
		return false;
	}

	if (FVector::Distance(TargetActor->GetActorLocation(), GetActorLocation()) > AttackRange)
	{
		return false;
	}

	LastAttackTimeSeconds = CurrentTime;
	UGameplayStatics::ApplyDamage(TargetActor, AttackDamage, GetController(), this, UDamageType::StaticClass());
	if (ABSPhase0Character* TargetCharacter = Cast<ABSPhase0Character>(TargetActor))
	{
		if (UBSSurvivorStateComponent* TargetState = TargetCharacter->GetSurvivorStateComponent())
		{
			TargetState->ApplyStress(18.0f);
			TargetState->SetHunted(true);
		}
	}

	if (NoiseEmitterComponent)
	{
		NoiseEmitterComponent->EmitNoise(EBSNoiseType::Combat, 0.55f, TEXT("InfectedAttack"));
	}

	return true;
}

void ABSInfectedCharacter::HandleDeath()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Emerald, TEXT("Infected neutralized."));
	}

	SetLifeSpan(2.0f);
}
