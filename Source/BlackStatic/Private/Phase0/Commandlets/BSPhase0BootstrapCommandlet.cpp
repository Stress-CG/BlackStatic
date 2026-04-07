#include "Phase0/Commandlets/BSPhase0BootstrapCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "CollisionQueryParams.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Phase0/Actors/BSExtractionPoint.h"
#include "Phase0/Actors/BSInfectedCharacter.h"
#include "Phase0/Actors/BSObjectivePickup.h"
#include "Phase0/Actors/BSStashActor.h"
#include "Phase0/Actors/BSTaskBoardActor.h"
#include "Phase0/Actors/BSWaterPowerObjectiveActor.h"
#include "Phase0/Data/BSItemDefinition.h"
#include "Phase0/Data/BSTaskDefinition.h"
#include "Phase0\Framework/BSPhase0GameMode.h"
#include "UObject/SavePackage.h"

#if WITH_EDITOR
#include "FileHelpers.h"
#endif

namespace
{
	static const FString ItemRoot = TEXT("/Game/Phase0/Data/Items");
	static const FString TaskRoot = TEXT("/Game/Phase0/Data/Tasks");
	static const FString PrototypeMapPath = TEXT("/Game/Phase0/Maps/MAP_Phase0_Prototype");
	static const FString MainMapPath = TEXT("/Game/MAP_TutorialRoad_P");
	static const FString FirstPersonMapPath = TEXT("/Game/FirstPerson/Lvl_FirstPerson");
	static const FString SpawnedActorPrefix = TEXT("BSP0_");

	struct FBootstrapMeshSpec
	{
		FString LabelSuffix;
		FVector LocalOffset = FVector::ZeroVector;
		FVector Scale = FVector::OneVector;
		FRotator LocalRotation = FRotator::ZeroRotator;
		float PlacementHeight = 80.0f;
	};

	struct FBootstrapMapLayout
	{
		bool bSpawnPrototypeShell = false;
		FVector TaskBoardOffset = FVector(180.0f, 0.0f, 0.0f);
		FVector StashOffset = FVector(360.0f, 110.0f, 0.0f);
		FVector ExtractionOffset = FVector(540.0f, -110.0f, 0.0f);
		FVector ObjectiveOffset = FVector(4100.0f, 0.0f, 0.0f);
		FVector BatteryOffset = FVector(3800.0f, -220.0f, 0.0f);
		FVector FilterOffset = FVector(4375.0f, 230.0f, 0.0f);
		FVector NavMeshCenterOffset = FVector(2200.0f, 0.0f, 200.0f);
		FVector NavMeshScale = FVector(20.0f, 12.0f, 4.0f);
		float ObjectiveDefenseRadius = 1200.0f;
		TArray<FBootstrapMeshSpec> SupportMeshes;
	};

	template <typename TAssetType>
	TAssetType* CreateOrLoadAsset(const FString& PackageName, const FString& AssetName)
	{
		UPackage* Package = CreatePackage(*PackageName);
		Package->FullyLoad();

		TAssetType* Asset = FindObject<TAssetType>(Package, *AssetName);
		if (!Asset)
		{
			Asset = NewObject<TAssetType>(Package, *AssetName, RF_Public | RF_Standalone);
			FAssetRegistryModule::AssetCreated(Asset);
		}

		return Asset;
	}

	bool SaveAsset(UObject* Asset)
	{
		if (!Asset)
		{
			return false;
		}

		UPackage* Package = Asset->GetPackage();
		Package->MarkPackageDirty();

		const FString FilePath = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;
		return UPackage::SavePackage(Package, Asset, *FilePath, SaveArgs);
	}

	FString ToObjectPath(const FString& AssetPath)
	{
		return FString::Printf(TEXT("%s.%s"), *AssetPath, *FPackageName::GetShortName(AssetPath));
	}

	bool IsPrototypeMapPath(const FString& MapPath)
	{
		return MapPath.Equals(PrototypeMapPath, ESearchCase::IgnoreCase);
	}

	FBootstrapMapLayout BuildLayoutForMap(const FString& MapPath)
	{
		FBootstrapMapLayout Layout;
		if (IsPrototypeMapPath(MapPath))
		{
			Layout.bSpawnPrototypeShell = true;
			Layout.SupportMeshes =
			{
				{ TEXT("SettlementWallA"), FVector(900.0f, 350.0f, 0.0f), FVector(2.0f, 0.15f, 2.0f), FRotator::ZeroRotator, 100.0f },
				{ TEXT("SettlementWallB"), FVector(900.0f, -350.0f, 0.0f), FVector(2.0f, 0.15f, 2.0f), FRotator::ZeroRotator, 100.0f },
				{ TEXT("RoadBlockA"), FVector(2350.0f, 260.0f, 0.0f), FVector(1.6f, 0.5f, 1.8f), FRotator::ZeroRotator, 90.0f },
				{ TEXT("RoadBlockB"), FVector(2600.0f, -260.0f, 0.0f), FVector(1.6f, 0.5f, 1.8f), FRotator::ZeroRotator, 90.0f },
				{ TEXT("SiteBarrierA"), FVector(3950.0f, 420.0f, 0.0f), FVector(2.4f, 0.25f, 2.2f), FRotator::ZeroRotator, 110.0f },
				{ TEXT("SiteBarrierB"), FVector(3950.0f, -420.0f, 0.0f), FVector(2.4f, 0.25f, 2.2f), FRotator::ZeroRotator, 110.0f }
			};
			return Layout;
		}

		if (MapPath.Equals(MainMapPath, ESearchCase::IgnoreCase))
		{
			Layout.TaskBoardOffset = FVector(250.0f, 180.0f, 0.0f);
			Layout.StashOffset = FVector(420.0f, -170.0f, 0.0f);
			Layout.ExtractionOffset = FVector(620.0f, 0.0f, 0.0f);
			Layout.ObjectiveOffset = FVector(5200.0f, 0.0f, 0.0f);
			Layout.BatteryOffset = FVector(4840.0f, -360.0f, 0.0f);
			Layout.FilterOffset = FVector(5520.0f, 320.0f, 0.0f);
			Layout.NavMeshCenterOffset = FVector(2900.0f, 0.0f, 240.0f);
			Layout.NavMeshScale = FVector(34.0f, 18.0f, 6.0f);
			Layout.ObjectiveDefenseRadius = 1500.0f;
			Layout.SupportMeshes =
			{
				{ TEXT("MainMapCheckpointA"), FVector(1700.0f, 260.0f, 0.0f), FVector(0.8f, 0.8f, 1.6f), FRotator::ZeroRotator, 90.0f },
				{ TEXT("MainMapCheckpointB"), FVector(1700.0f, -260.0f, 0.0f), FVector(0.8f, 0.8f, 1.6f), FRotator::ZeroRotator, 90.0f },
				{ TEXT("MainMapObjectiveBarricadeA"), FVector(5050.0f, 420.0f, 0.0f), FVector(1.4f, 0.25f, 1.8f), FRotator::ZeroRotator, 90.0f },
				{ TEXT("MainMapObjectiveBarricadeB"), FVector(5050.0f, -420.0f, 0.0f), FVector(1.4f, 0.25f, 1.8f), FRotator::ZeroRotator, 90.0f }
			};
			return Layout;
		}

		Layout.TaskBoardOffset = FVector(220.0f, 120.0f, 0.0f);
		Layout.StashOffset = FVector(360.0f, -120.0f, 0.0f);
		Layout.ExtractionOffset = FVector(540.0f, 0.0f, 0.0f);
		Layout.ObjectiveOffset = FVector(2800.0f, 0.0f, 0.0f);
		Layout.BatteryOffset = FVector(2520.0f, -240.0f, 0.0f);
		Layout.FilterOffset = FVector(3080.0f, 210.0f, 0.0f);
		Layout.NavMeshCenterOffset = FVector(1550.0f, 0.0f, 200.0f);
		Layout.NavMeshScale = FVector(18.0f, 12.0f, 4.0f);
		Layout.ObjectiveDefenseRadius = 1000.0f;
		Layout.SupportMeshes =
		{
			{ TEXT("ArenaGateA"), FVector(1200.0f, 280.0f, 0.0f), FVector(0.7f, 0.3f, 1.4f), FRotator::ZeroRotator, 85.0f },
			{ TEXT("ArenaGateB"), FVector(1200.0f, -280.0f, 0.0f), FVector(0.7f, 0.3f, 1.4f), FRotator::ZeroRotator, 85.0f }
		};
		return Layout;
	}

	TArray<FString> ResolveTargetMaps(const FString& Params)
	{
		TArray<FString> TargetMaps;

		FString ExplicitTargetMapList;
		if (FParse::Value(*Params, TEXT("TargetMap="), ExplicitTargetMapList))
		{
			ExplicitTargetMapList.ParseIntoArray(TargetMaps, TEXT(","), true);
			for (FString& TargetMap : TargetMaps)
			{
				TargetMap.TrimStartAndEndInline();
			}

			TargetMaps.RemoveAll([](const FString& Value)
			{
				return Value.IsEmpty();
			});
			return TargetMaps;
		}

		const bool bPrototypeOnly = FParse::Param(*Params, TEXT("PrototypeOnly"));
		const bool bMainMapsOnly = FParse::Param(*Params, TEXT("MainMapsOnly"));

		if (!bMainMapsOnly)
		{
			TargetMaps.Add(PrototypeMapPath);
		}

		if (!bPrototypeOnly)
		{
			TargetMaps.Add(MainMapPath);
			TargetMaps.Add(FirstPersonMapPath);
		}

		return TargetMaps;
	}

#if WITH_EDITOR
	void DestroyExistingBootstrapActors(UWorld* World)
	{
		if (!World)
		{
			return;
		}

		TArray<AActor*> ActorsToDestroy;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor)
			{
				continue;
			}

			if (Actor->GetActorLabel().StartsWith(SpawnedActorPrefix))
			{
				ActorsToDestroy.Add(Actor);
			}
		}

		for (AActor* Actor : ActorsToDestroy)
		{
			World->EditorDestroyActor(Actor, true);
		}
	}

	template <typename TActorType>
	TActorType* SpawnPhase0Actor(UWorld* World, UClass* ActorClass, const FString& Label, const FVector& Location, const FRotator& Rotation)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		TActorType* Actor = ActorClass
			? World->SpawnActor<TActorType>(ActorClass, Location, Rotation, SpawnParameters)
			: World->SpawnActor<TActorType>(Location, Rotation, SpawnParameters);
		if (!Actor)
		{
			return nullptr;
		}

		Actor->SetActorLabel(Label);
		return Actor;
	}

	AStaticMeshActor* SpawnMeshActor(UWorld* World, UStaticMesh* StaticMesh, const FString& Label, const FVector& Location, const FVector& Scale, const FRotator& Rotation = FRotator::ZeroRotator)
	{
		AStaticMeshActor* Actor = SpawnPhase0Actor<AStaticMeshActor>(World, nullptr, Label, Location, Rotation);
		if (!Actor)
		{
			return nullptr;
		}

		Actor->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);
		Actor->SetActorScale3D(Scale);
		Actor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
		return Actor;
	}

	APlayerStart* FindOrCreatePlayerStart(UWorld* World)
	{
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			if (APlayerStart* Existing = *It)
			{
				return Existing;
			}
		}

		return SpawnPhase0Actor<APlayerStart>(World, nullptr, SpawnedActorPrefix + TEXT("PlayerStart"), FVector(0.0f, 0.0f, 120.0f), FRotator::ZeroRotator);
	}

	FVector ResolveGroundLocation(UWorld* World, const FVector& DesiredLocation, const float HeightOffset)
	{
		if (!World)
		{
			return DesiredLocation;
		}

		const FVector TraceStart = DesiredLocation + FVector(0.0f, 0.0f, 4000.0f);
		const FVector TraceEnd = DesiredLocation - FVector(0.0f, 0.0f, 4000.0f);
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BSPhase0GroundTrace), true);
		QueryParams.bTraceComplex = false;

		FHitResult HitResult;
		if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
		{
			return HitResult.ImpactPoint + FVector(0.0f, 0.0f, HeightOffset);
		}

		return DesiredLocation + FVector(0.0f, 0.0f, HeightOffset);
	}

	FVector ResolveLocalPlacement(UWorld* World, const FTransform& AnchorTransform, const FVector& LocalOffset, const float HeightOffset)
	{
		return ResolveGroundLocation(World, AnchorTransform.TransformPosition(LocalOffset), HeightOffset);
	}

	UWorld* LoadOrCreateMap(const FString& MapPath)
	{
		if (FPackageName::DoesPackageExist(MapPath))
		{
			const FString MapFilename = FPackageName::LongPackageNameToFilename(MapPath, FPackageName::GetMapPackageExtension());
			return UEditorLoadingAndSavingUtils::LoadMap(MapFilename);
		}

		if (IsPrototypeMapPath(MapPath))
		{
			return UEditorLoadingAndSavingUtils::NewBlankMap(false);
		}

		return nullptr;
	}

	bool BootstrapMap(UWorld* TargetWorld, const FString& TargetMapPath, UBSItemDefinition* BatteryDefinition, UBSItemDefinition* FilterDefinition, UBSTaskDefinition* WaterPowerTask)
	{
		if (!TargetWorld || !BatteryDefinition || !FilterDefinition || !WaterPowerTask)
		{
			return false;
		}

		UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
		UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		if (!PlaneMesh || !CubeMesh)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load required engine meshes for %s."), *TargetMapPath);
			return false;
		}

		DestroyExistingBootstrapActors(TargetWorld);

		if (TargetWorld->GetWorldSettings())
		{
			TargetWorld->GetWorldSettings()->Modify();
			TargetWorld->GetWorldSettings()->DefaultGameMode = ABSPhase0GameMode::StaticClass();
		}

		APlayerStart* PlayerStart = FindOrCreatePlayerStart(TargetWorld);
		if (!PlayerStart)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to locate or create a PlayerStart for %s."), *TargetMapPath);
			return false;
		}

		const FTransform AnchorTransform(PlayerStart->GetActorRotation(), PlayerStart->GetActorLocation());
		const FBootstrapMapLayout Layout = BuildLayoutForMap(TargetMapPath);

		if (Layout.bSpawnPrototypeShell)
		{
			const FVector GroundLocation = ResolveGroundLocation(TargetWorld, AnchorTransform.TransformPosition(FVector(2200.0f, 0.0f, -10.0f)), 0.0f);
			SpawnMeshActor(TargetWorld, PlaneMesh, SpawnedActorPrefix + TEXT("Ground"), GroundLocation, FVector(45.0f, 18.0f, 1.0f), AnchorTransform.GetRotation().Rotator());

			SpawnPhase0Actor<ADirectionalLight>(
				TargetWorld,
				nullptr,
				SpawnedActorPrefix + TEXT("DirectionalLight"),
				ResolveGroundLocation(TargetWorld, AnchorTransform.TransformPosition(FVector(0.0f, 0.0f, 600.0f)), 600.0f),
				FRotator(-50.0f, 35.0f, 0.0f));
			SpawnPhase0Actor<ASkyLight>(
				TargetWorld,
				nullptr,
				SpawnedActorPrefix + TEXT("SkyLight"),
				ResolveGroundLocation(TargetWorld, AnchorTransform.TransformPosition(FVector(0.0f, 0.0f, 250.0f)), 250.0f),
				FRotator::ZeroRotator);
		}

		ANavMeshBoundsVolume* NavBounds = SpawnPhase0Actor<ANavMeshBoundsVolume>(
			TargetWorld,
			nullptr,
			SpawnedActorPrefix + TEXT("NavMeshBounds"),
			ResolveLocalPlacement(TargetWorld, AnchorTransform, Layout.NavMeshCenterOffset, Layout.NavMeshCenterOffset.Z),
			FRotator::ZeroRotator);
		if (NavBounds)
		{
			NavBounds->SetActorScale3D(Layout.NavMeshScale);
		}

		for (const FBootstrapMeshSpec& MeshSpec : Layout.SupportMeshes)
		{
			SpawnMeshActor(
				TargetWorld,
				CubeMesh,
				SpawnedActorPrefix + MeshSpec.LabelSuffix,
				ResolveLocalPlacement(TargetWorld, AnchorTransform, MeshSpec.LocalOffset, MeshSpec.PlacementHeight),
				MeshSpec.Scale,
				AnchorTransform.GetRotation().Rotator() + MeshSpec.LocalRotation);
		}

		ABSTaskBoardActor* TaskBoard = SpawnPhase0Actor<ABSTaskBoardActor>(
			TargetWorld,
			nullptr,
			SpawnedActorPrefix + TEXT("TaskBoard"),
			ResolveLocalPlacement(TargetWorld, AnchorTransform, Layout.TaskBoardOffset, 40.0f),
			AnchorTransform.GetRotation().Rotator());
		ABSStashActor* Stash = SpawnPhase0Actor<ABSStashActor>(
			TargetWorld,
			nullptr,
			SpawnedActorPrefix + TEXT("Stash"),
			ResolveLocalPlacement(TargetWorld, AnchorTransform, Layout.StashOffset, 40.0f),
			AnchorTransform.GetRotation().Rotator());
		ABSExtractionPoint* Extraction = SpawnPhase0Actor<ABSExtractionPoint>(
			TargetWorld,
			nullptr,
			SpawnedActorPrefix + TEXT("Extraction"),
			ResolveLocalPlacement(TargetWorld, AnchorTransform, Layout.ExtractionOffset, 10.0f),
			AnchorTransform.GetRotation().Rotator());
		ABSWaterPowerObjectiveActor* ObjectiveActor = SpawnPhase0Actor<ABSWaterPowerObjectiveActor>(
			TargetWorld,
			nullptr,
			SpawnedActorPrefix + TEXT("WaterPowerObjective"),
			ResolveLocalPlacement(TargetWorld, AnchorTransform, Layout.ObjectiveOffset, 30.0f),
			AnchorTransform.GetRotation().Rotator());
		ABSObjectivePickup* BatteryPickup = SpawnPhase0Actor<ABSObjectivePickup>(
			TargetWorld,
			nullptr,
			SpawnedActorPrefix + TEXT("BatteryPickup"),
			ResolveLocalPlacement(TargetWorld, AnchorTransform, Layout.BatteryOffset, 35.0f),
			AnchorTransform.GetRotation().Rotator());
		ABSObjectivePickup* FilterPickup = SpawnPhase0Actor<ABSObjectivePickup>(
			TargetWorld,
			nullptr,
			SpawnedActorPrefix + TEXT("FilterPickup"),
			ResolveLocalPlacement(TargetWorld, AnchorTransform, Layout.FilterOffset, 35.0f),
			AnchorTransform.GetRotation().Rotator());

		if (!TaskBoard || !Stash || !Extraction || !ObjectiveActor || !BatteryPickup || !FilterPickup)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to place one or more Phase 0 actors into %s."), *TargetMapPath);
			return false;
		}

		TaskBoard->TaskDefinition = WaterPowerTask;
		TaskBoard->LinkedObjective = ObjectiveActor;

		ObjectiveActor->TaskDefinition = WaterPowerTask;
		ObjectiveActor->InfectedClass = ABSInfectedCharacter::StaticClass();
		ObjectiveActor->ObjectiveDefenseRadius = Layout.ObjectiveDefenseRadius;

		BatteryPickup->ItemDefinition = BatteryDefinition;
		FilterPickup->ItemDefinition = FilterDefinition;

		return true;
	}
#endif
}

UBSPhase0BootstrapCommandlet::UBSPhase0BootstrapCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
}

int32 UBSPhase0BootstrapCommandlet::Main(const FString& Params)
{
#if !WITH_EDITOR
	return 1;
#else
	const bool bCreateAssets = !FParse::Param(*Params, TEXT("MapOnly"));
	const bool bCreateMaps = !FParse::Param(*Params, TEXT("AssetsOnly"));

	IFileManager::Get().MakeDirectory(*(FPaths::ProjectContentDir() / TEXT("Phase0/Data/Items")), true);
	IFileManager::Get().MakeDirectory(*(FPaths::ProjectContentDir() / TEXT("Phase0/Data/Tasks")), true);
	IFileManager::Get().MakeDirectory(*(FPaths::ProjectContentDir() / TEXT("Phase0/Maps")), true);

	UBSItemDefinition* BatteryDefinition = nullptr;
	UBSItemDefinition* FilterDefinition = nullptr;
	UBSItemDefinition* WiresDefinition = nullptr;
	UBSItemDefinition* BandageDefinition = nullptr;
	UBSItemDefinition* WaterDefinition = nullptr;

	if (bCreateAssets)
	{
		BatteryDefinition = CreateOrLoadAsset<UBSItemDefinition>(ItemRoot / TEXT("DA_BS_Battery"), TEXT("DA_BS_Battery"));
		BatteryDefinition->ItemId = TEXT("Battery");
		BatteryDefinition->DisplayName = FText::FromString(TEXT("Battery"));
		BatteryDefinition->Description = FText::FromString(TEXT("A restored power cell needed for water and power repairs."));
		BatteryDefinition->Category = EBSItemCategory::Quest;
		BatteryDefinition->BarterValue = 35;
		BatteryDefinition->SettlementValue = 20;
		BatteryDefinition->bQuestCritical = true;
		SaveAsset(BatteryDefinition);

		FilterDefinition = CreateOrLoadAsset<UBSItemDefinition>(ItemRoot / TEXT("DA_BS_Filter"), TEXT("DA_BS_Filter"));
		FilterDefinition->ItemId = TEXT("Filter");
		FilterDefinition->DisplayName = FText::FromString(TEXT("Filter"));
		FilterDefinition->Description = FText::FromString(TEXT("A water filtration cartridge required for settlement pump repairs."));
		FilterDefinition->Category = EBSItemCategory::Quest;
		FilterDefinition->BarterValue = 25;
		FilterDefinition->SettlementValue = 20;
		FilterDefinition->bQuestCritical = true;
		SaveAsset(FilterDefinition);

		WiresDefinition = CreateOrLoadAsset<UBSItemDefinition>(ItemRoot / TEXT("DA_BS_Wires"), TEXT("DA_BS_Wires"));
		WiresDefinition->ItemId = TEXT("Wires");
		WiresDefinition->DisplayName = FText::FromString(TEXT("Wires"));
		WiresDefinition->Description = FText::FromString(TEXT("Multi-use electrical wire that can be traded or used for infrastructure upgrades."));
		WiresDefinition->Category = EBSItemCategory::Settlement;
		WiresDefinition->USDValue = 15;
		WiresDefinition->BarterValue = 12;
		WiresDefinition->SettlementValue = 10;
		SaveAsset(WiresDefinition);

		BandageDefinition = CreateOrLoadAsset<UBSItemDefinition>(ItemRoot / TEXT("DA_BS_Bandage"), TEXT("DA_BS_Bandage"));
		BandageDefinition->ItemId = TEXT("Bandage");
		BandageDefinition->DisplayName = FText::FromString(TEXT("Bandage"));
		BandageDefinition->Description = FText::FromString(TEXT("Basic wound treatment. Commonly sold for USD."));
		BandageDefinition->Category = EBSItemCategory::Medical;
		BandageDefinition->USDValue = 18;
		BandageDefinition->bConsumable = true;
		SaveAsset(BandageDefinition);

		WaterDefinition = CreateOrLoadAsset<UBSItemDefinition>(ItemRoot / TEXT("DA_BS_WaterBottle"), TEXT("DA_BS_WaterBottle"));
		WaterDefinition->ItemId = TEXT("WaterBottle");
		WaterDefinition->DisplayName = FText::FromString(TEXT("Water Bottle"));
		WaterDefinition->Description = FText::FromString(TEXT("Basic hydration supply bought with USD inside settlements."));
		WaterDefinition->Category = EBSItemCategory::Water;
		WaterDefinition->USDValue = 10;
		WaterDefinition->bConsumable = true;
		SaveAsset(WaterDefinition);

		UBSTaskDefinition* WaterPowerTask = CreateOrLoadAsset<UBSTaskDefinition>(TaskRoot / TEXT("DA_BS_WaterPowerRestoration"), TEXT("DA_BS_WaterPowerRestoration"));
		WaterPowerTask->TaskId = TEXT("WaterPowerRestoration");
		WaterPowerTask->DisplayName = FText::FromString(TEXT("Water & Power Restoration"));
		WaterPowerTask->Description = FText::FromString(TEXT("Leave the settlement, reach the maintenance site, recover a battery and filter, restore utilities, and extract alive."));
		WaterPowerTask->ObjectiveType = EBSObjectiveType::WaterPowerRestoration;
		WaterPowerTask->RequiredItems =
		{
			{ BatteryDefinition, BatteryDefinition->ResolveItemId(), 1 },
			{ FilterDefinition, FilterDefinition->ResolveItemId(), 1 }
		};
		WaterPowerTask->Reward.USD = 120;
		WaterPowerTask->Reward.Reputation = 25.0f;
		WaterPowerTask->Reward.VendorTrust = 12.0f;
		WaterPowerTask->Reward.SettlementPower = 1;
		WaterPowerTask->Reward.SettlementWater = 1;
		WaterPowerTask->Reward.SettlementInfrastructure = 1;
		WaterPowerTask->BaseThreatCount = 4;
		WaterPowerTask->bRepeatable = true;
		WaterPowerTask->bRequireExtractionAfterSiteCompletion = true;
		SaveAsset(WaterPowerTask);
	}

	if (!bCreateMaps)
	{
		return 0;
	}

	if (!BatteryDefinition)
	{
		BatteryDefinition = LoadObject<UBSItemDefinition>(nullptr, *ToObjectPath(TEXT("/Game/Phase0/Data/Items/DA_BS_Battery")));
	}

	if (!FilterDefinition)
	{
		FilterDefinition = LoadObject<UBSItemDefinition>(nullptr, *ToObjectPath(TEXT("/Game/Phase0/Data/Items/DA_BS_Filter")));
	}

	UBSTaskDefinition* WaterPowerTask = LoadObject<UBSTaskDefinition>(nullptr, *ToObjectPath(TEXT("/Game/Phase0/Data/Tasks/DA_BS_WaterPowerRestoration")));
	if (!BatteryDefinition || !FilterDefinition || !WaterPowerTask)
	{
		UE_LOG(LogTemp, Error, TEXT("Phase 0 assets are missing. Run the bootstrap with asset creation enabled."));
		return 1;
	}

	const TArray<FString> TargetMaps = ResolveTargetMaps(Params);
	if (TargetMaps.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No target maps were resolved for the Phase 0 bootstrap."));
		return 1;
	}

	TArray<FString> UpdatedMaps;
	for (const FString& TargetMapPath : TargetMaps)
	{
		UWorld* TargetWorld = LoadOrCreateMap(TargetMapPath);
		if (!TargetWorld)
		{
			UE_LOG(LogTemp, Warning, TEXT("Skipping %s because it could not be loaded or created."), *TargetMapPath);
			continue;
		}

		if (!BootstrapMap(TargetWorld, TargetMapPath, BatteryDefinition, FilterDefinition, WaterPowerTask))
		{
			return 1;
		}

		if (!UEditorLoadingAndSavingUtils::SaveMap(TargetWorld, TargetMapPath))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to save map %s."), *TargetMapPath);
			return 1;
		}

		UpdatedMaps.Add(TargetMapPath);
		UE_LOG(LogTemp, Display, TEXT("Phase 0 content applied to %s"), *TargetMapPath);
	}

	if (UpdatedMaps.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Phase 0 bootstrap did not update any maps."));
		return 1;
	}

	UE_LOG(LogTemp, Display, TEXT("Phase 0 content is ready on %d map(s). Primary gameplay map: %s"), UpdatedMaps.Num(), *MainMapPath);
	return 0;
#endif
}
