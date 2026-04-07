#include "Phase0/Actors/BSPhase0Character.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputCoreTypes.h"
#include "Phase0/Components/BSInventoryComponent.h"
#include "Phase0/Components/BSNoiseEmitterComponent.h"
#include "Phase0/Components/BSSurvivorStateComponent.h"
#include "Phase0/Components/BSTaskComponent.h"
#include "Phase0/Framework/BSPhase0PlayerController.h"
#include "Phase0/Interfaces/BSInteractableInterface.h"
#include "Phase0/Framework/BSPhase0GameMode.h"
#include "UObject/ConstructorHelpers.h"

ABSPhase0Character::ABSPhase0Character()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	bUseControllerRotationYaw = true;

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-5.0f, 0.0f, 64.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	InventoryComponent = CreateDefaultSubobject<UBSInventoryComponent>(TEXT("InventoryComponent"));
	SurvivorStateComponent = CreateDefaultSubobject<UBSSurvivorStateComponent>(TEXT("SurvivorStateComponent"));
	NoiseEmitterComponent = CreateDefaultSubobject<UBSNoiseEmitterComponent>(TEXT("NoiseEmitterComponent"));
	TaskComponent = CreateDefaultSubobject<UBSTaskComponent>(TEXT("TaskComponent"));

	GetCharacterMovement()->MaxWalkSpeed = IntentionalWalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 150.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1800.0f;
	GetCharacterMovement()->AirControl = 0.2f;

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultMappingFinder(TEXT("/Game/Input/IMC_Default.IMC_Default"));
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MouseMappingFinder(TEXT("/Game/Input/IMC_MouseLook.IMC_MouseLook"));
	static ConstructorHelpers::FObjectFinder<UInputAction> MoveActionFinder(TEXT("/Game/Input/Actions/IA_Move.IA_Move"));
	static ConstructorHelpers::FObjectFinder<UInputAction> LookActionFinder(TEXT("/Game/Input/Actions/IA_Look.IA_Look"));
	static ConstructorHelpers::FObjectFinder<UInputAction> MouseLookActionFinder(TEXT("/Game/Input/Actions/IA_MouseLook.IA_MouseLook"));
	static ConstructorHelpers::FObjectFinder<UInputAction> JumpActionFinder(TEXT("/Game/Input/Actions/IA_Jump.IA_Jump"));
	static ConstructorHelpers::FObjectFinder<UInputAction> CrouchActionFinder(TEXT("/Game/Input/Actions/IA_Crouch.IA_Crouch"));
	static ConstructorHelpers::FObjectFinder<UInputAction> RunActionFinder(TEXT("/Game/Input/Actions/IA_Run.IA_Run"));

	DefaultInputMapping = DefaultMappingFinder.Succeeded() ? DefaultMappingFinder.Object : nullptr;
	MouseLookMapping = MouseMappingFinder.Succeeded() ? MouseMappingFinder.Object : nullptr;
	MoveInputAction = MoveActionFinder.Succeeded() ? MoveActionFinder.Object : nullptr;
	LookInputAction = LookActionFinder.Succeeded() ? LookActionFinder.Object : nullptr;
	MouseLookInputAction = MouseLookActionFinder.Succeeded() ? MouseLookActionFinder.Object : nullptr;
	JumpInputAction = JumpActionFinder.Succeeded() ? JumpActionFinder.Object : nullptr;
	CrouchInputAction = CrouchActionFinder.Succeeded() ? CrouchActionFinder.Object : nullptr;
	RunInputAction = RunActionFinder.Succeeded() ? RunActionFinder.Object : nullptr;
}

void ABSPhase0Character::BeginPlay()
{
	Super::BeginPlay();

	if (NoiseEmitterComponent)
	{
		NoiseEmitterComponent->EmittedFaction = Faction;
	}

	if (SurvivorStateComponent)
	{
		SurvivorStateComponent->OnDeath.AddDynamic(this, &ABSPhase0Character::HandleSurvivorDeath);
	}

	BaseCameraRelativeLocation = FirstPersonCameraComponent ? FirstPersonCameraComponent->GetRelativeLocation() : FVector::ZeroVector;
	BaseCameraFOV = FirstPersonCameraComponent ? FirstPersonCameraComponent->FieldOfView : 90.0f;
	ApplyInputMappings();
	UpdateMovementSpeedFromVitals();
	UpdateFocusedInteractable();
	UpdateStressCameraEffects(0.0f);
}

void ABSPhase0Character::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateMovementSpeedFromVitals();
	UpdateFocusedInteractable();
	UpdateStressCameraEffects(DeltaSeconds);

	if (!NoiseEmitterComponent || !SurvivorStateComponent || !SurvivorStateComponent->IsAlive())
	{
		return;
	}

	MovementNoiseAccumulator += DeltaSeconds;
	if (MovementNoiseAccumulator < MovementNoiseIntervalSeconds)
	{
		return;
	}

	MovementNoiseAccumulator = 0.0f;
	const float Speed = GetVelocity().Size2D();
	const float MaxWalkSpeed = bSprintRequested ? SprintSpeed : IntentionalWalkSpeed;
	if (Speed > 10.0f)
	{
		NoiseEmitterComponent->EmitMovementNoise(Speed, MaxWalkSpeed, bIsCrouched, bSprintRequested);
	}
}

void ABSPhase0Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveInputAction)
		{
			EnhancedInputComponent->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &ABSPhase0Character::InputMove);
		}

		if (LookInputAction)
		{
			EnhancedInputComponent->BindAction(LookInputAction, ETriggerEvent::Triggered, this, &ABSPhase0Character::InputLook);
		}

		if (MouseLookInputAction)
		{
			EnhancedInputComponent->BindAction(MouseLookInputAction, ETriggerEvent::Triggered, this, &ABSPhase0Character::InputLook);
		}

		if (JumpInputAction)
		{
			EnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Started, this, &ABSPhase0Character::InputJumpStarted);
			EnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Completed, this, &ABSPhase0Character::InputJumpStopped);
		}

		if (CrouchInputAction)
		{
			EnhancedInputComponent->BindAction(CrouchInputAction, ETriggerEvent::Started, this, &ABSPhase0Character::InputToggleCrouch);
		}

		if (RunInputAction)
		{
			EnhancedInputComponent->BindAction(RunInputAction, ETriggerEvent::Started, this, &ABSPhase0Character::InputSprintStarted);
			EnhancedInputComponent->BindAction(RunInputAction, ETriggerEvent::Completed, this, &ABSPhase0Character::InputSprintStopped);
			EnhancedInputComponent->BindAction(RunInputAction, ETriggerEvent::Canceled, this, &ABSPhase0Character::InputSprintStopped);
		}

	}

	PlayerInputComponent->BindKey(EKeys::E, IE_Pressed, this, &ABSPhase0Character::TryInteract);
	PlayerInputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &ABSPhase0Character::InputToggleInventory);
}

float ABSPhase0Character::TakeDamage(const float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!SurvivorStateComponent)
	{
		return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	}

	return SurvivorStateComponent->ApplyWorldDamage(DamageAmount);
}

void ABSPhase0Character::TryInteract()
{
	UpdateFocusedInteractable();

	if (!Controller || !FocusedInteractableActor.IsValid())
	{
		return;
	}

	AActor* Interactable = FocusedInteractableActor.Get();
	if (Interactable && Interactable->GetClass()->ImplementsInterface(UBSInteractableInterface::StaticClass()) && IBSInteractableInterface::Execute_CanInteract(Interactable, this))
	{
		IBSInteractableInterface::Execute_Interact(Interactable, this);
		UpdateFocusedInteractable();
	}
}

void ABSPhase0Character::SetSprintEnabled(const bool bEnabled)
{
	bSprintRequested = bEnabled && SurvivorStateComponent && SurvivorStateComponent->IsAlive();
	if (SurvivorStateComponent)
	{
		SurvivorStateComponent->SetSprinting(bSprintRequested);
	}

	UpdateMovementSpeedFromVitals();
}

void ABSPhase0Character::ReportCombatNoise(const float Loudness)
{
	if (NoiseEmitterComponent)
	{
		NoiseEmitterComponent->EmitNoise(EBSNoiseType::Combat, Loudness, TEXT("Combat"));
	}
}

UBSInventoryComponent* ABSPhase0Character::GetInventoryComponent() const
{
	return InventoryComponent;
}

UBSSurvivorStateComponent* ABSPhase0Character::GetSurvivorStateComponent() const
{
	return SurvivorStateComponent;
}

UBSTaskComponent* ABSPhase0Character::GetTaskComponent() const
{
	return TaskComponent;
}

FText ABSPhase0Character::GetCurrentInteractionPrompt() const
{
	return FocusedInteractionPrompt;
}

AActor* ABSPhase0Character::GetFocusedInteractableActor() const
{
	return FocusedInteractableActor.Get();
}

void ABSPhase0Character::HandleSurvivorDeath()
{
	DisableInput(Cast<APlayerController>(Controller));
	SetSprintEnabled(false);

	if (ABSPhase0GameMode* Phase0GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ABSPhase0GameMode>() : nullptr)
	{
		Phase0GameMode->HandleSurvivorDeath(this);
	}
}

void ABSPhase0Character::UpdateFocusedInteractable()
{
	FocusedInteractableActor = nullptr;
	FocusedInteractionPrompt = FText::GetEmpty();

	if (!Controller || !GetWorld())
	{
		return;
	}

	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const FVector TraceEnd = ViewLocation + (ViewRotation.Vector() * InteractionDistance);
	FHitResult Hit;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(Phase0Interact), true, this);

	if (!GetWorld()->LineTraceSingleByChannel(Hit, ViewLocation, TraceEnd, ECC_Visibility, QueryParams))
	{
		return;
	}

	AActor* HitActor = Hit.GetActor();
	if (!HitActor || !HitActor->GetClass()->ImplementsInterface(UBSInteractableInterface::StaticClass()))
	{
		return;
	}

	if (!IBSInteractableInterface::Execute_CanInteract(HitActor, this))
	{
		return;
	}

	FocusedInteractableActor = HitActor;
	FocusedInteractionPrompt = IBSInteractableInterface::Execute_GetInteractionPrompt(HitActor);
}

void ABSPhase0Character::UpdateStressCameraEffects(const float DeltaSeconds)
{
	if (!FirstPersonCameraComponent || !SurvivorStateComponent)
	{
		return;
	}

	const float StressStrength = FMath::Clamp(SurvivorStateComponent->GetVisionDisruptionStrength(), 0.0f, 1.0f);
	StressVisualTimeSeconds += DeltaSeconds * (1.0f + (StressStrength * 2.5f));

	const float Pulse = FMath::Sin(StressVisualTimeSeconds * 5.0f) * StressStrength;
	const FVector CameraOffset(0.0f, FMath::Sin(StressVisualTimeSeconds * 2.3f) * StressStrength * 1.5f, FMath::Cos(StressVisualTimeSeconds * 3.1f) * StressStrength * 1.0f);
	FirstPersonCameraComponent->SetRelativeLocation(BaseCameraRelativeLocation + CameraOffset);
	FirstPersonCameraComponent->SetFieldOfView(BaseCameraFOV + (StressStrength * 3.5f) + (Pulse * 1.5f));

	FPostProcessSettings& PostProcess = FirstPersonCameraComponent->PostProcessSettings;
	PostProcess.bOverride_VignetteIntensity = true;
	PostProcess.bOverride_SceneFringeIntensity = true;
	PostProcess.bOverride_ColorSaturation = true;
	PostProcess.VignetteIntensity = FMath::Lerp(0.35f, 0.75f, StressStrength);
	PostProcess.SceneFringeIntensity = FMath::Lerp(0.0f, 3.0f, StressStrength);
	PostProcess.ColorSaturation = FVector4(FMath::Lerp(1.0f, 0.72f, StressStrength), FMath::Lerp(1.0f, 0.72f, StressStrength), FMath::Lerp(1.0f, 0.72f, StressStrength), 1.0f);
}

void ABSPhase0Character::ApplyInputMappings()
{
	APlayerController* PC = Cast<APlayerController>(Controller);
	if (!PC)
	{
		return;
	}

	if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (DefaultInputMapping)
			{
				InputSubsystem->AddMappingContext(DefaultInputMapping, 0);
			}
			if (MouseLookMapping)
			{
				InputSubsystem->AddMappingContext(MouseLookMapping, 1);
			}
		}
	}
}

void ABSPhase0Character::UpdateMovementSpeedFromVitals()
{
	if (!SurvivorStateComponent)
	{
		return;
	}

	const float BaseSpeed = bSprintRequested ? SprintSpeed : IntentionalWalkSpeed;
	GetCharacterMovement()->MaxWalkSpeed = BaseSpeed * SurvivorStateComponent->GetMovementSpeedMultiplier();
}

void ABSPhase0Character::InputMove(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	if (!Controller)
	{
		return;
	}

	const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ABSPhase0Character::InputLook(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxis.X);
	AddControllerPitchInput(LookAxis.Y);
}

void ABSPhase0Character::InputJumpStarted()
{
	Jump();
}

void ABSPhase0Character::InputJumpStopped()
{
	StopJumping();
}

void ABSPhase0Character::InputToggleCrouch()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABSPhase0Character::InputSprintStarted()
{
	SetSprintEnabled(true);
}

void ABSPhase0Character::InputSprintStopped()
{
	SetSprintEnabled(false);
}

void ABSPhase0Character::InputToggleInventory()
{
	if (ABSPhase0PlayerController* Phase0Controller = Cast<ABSPhase0PlayerController>(Controller))
	{
		Phase0Controller->ToggleBackpack();
	}
}
