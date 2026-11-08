#include "FarmPlayerController.h"
#include "FarmBin.h"
#include "FarmBuilding.h"
#include "FarmAnimal.h"
#include "FarmCharacter.h"
#include "FarmGameMode.h"
#include "FarmAudio.h"
#include "FarmInventoryComponent.h"
#include "FarmInventoryWidget.h"
#include "FarmMenuWidget.h"
#include "FarmShop.h"
#include "FarmShopWidget.h"
#include "FarmItems.h"
#include "FarmPlot.h"
#include "FarmPortal.h"
#include "FarmRecipes.h"
#include "FarmResourceNode.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Kismet/KismetSystemLibrary.h"

AFarmPlayerController::AFarmPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFarmPlayerController::BeginPlay()
{
	Super::BeginPlay();
	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());

	if (GEngine && GEngine->GameViewport)
	{
		SAssignNew(InventoryWidget, SFarmInventoryWidget).PlayerController(this);
		GEngine->GameViewport->AddViewportWidgetContent(InventoryWidget.ToSharedRef());
		InventoryWidget->SetVisibility(EVisibility::Collapsed);

		SAssignNew(MenuWidget, SFarmMenuWidget).PlayerController(this);
		GEngine->GameViewport->AddViewportWidgetContent(MenuWidget.ToSharedRef());
		MenuWidget->SetVisibility(EVisibility::Collapsed);
		ShowMenu(true);
	}
}

void AFarmPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &AFarmPlayerController::Interact);
	InputComponent->BindAction(TEXT("CycleSeed"), IE_Pressed, this, &AFarmPlayerController::CycleSeed);
	InputComponent->BindAction(TEXT("BuySeed"), IE_Pressed, this, &AFarmPlayerController::BuySeed);
	InputComponent->BindAction(TEXT("SaveGame"), IE_Pressed, this, &AFarmPlayerController::SaveGame);
	InputComponent->BindAction(TEXT("LoadGame"), IE_Pressed, this, &AFarmPlayerController::LoadGame);
	InputComponent->BindAction(TEXT("Sleep"), IE_Pressed, this, &AFarmPlayerController::Sleep);
	InputComponent->BindAction(TEXT("Attack"), IE_Pressed, this, &AFarmPlayerController::Attack);
	InputComponent->BindAction(TEXT("Build"), IE_Pressed, this, &AFarmPlayerController::ToggleBuildMode);
	InputComponent->BindAction(TEXT("CraftMenu"), IE_Pressed, this, &AFarmPlayerController::ToggleCraftMenu);
	InputComponent->BindAction(TEXT("Inventory"), IE_Pressed, this, &AFarmPlayerController::ToggleInventory);
	InputComponent->BindAction(TEXT("Eat"), IE_Pressed, this, &AFarmPlayerController::EatFood);
	InputComponent->BindAction(TEXT("Cancel"), IE_Pressed, this, &AFarmPlayerController::CancelMode);

	InputComponent->BindAction(TEXT("Craft1"), IE_Pressed, this, &AFarmPlayerController::Craft1);
	InputComponent->BindAction(TEXT("Craft2"), IE_Pressed, this, &AFarmPlayerController::Craft2);
	InputComponent->BindAction(TEXT("Craft3"), IE_Pressed, this, &AFarmPlayerController::Craft3);
	InputComponent->BindAction(TEXT("Craft4"), IE_Pressed, this, &AFarmPlayerController::Craft4);
	InputComponent->BindAction(TEXT("Craft5"), IE_Pressed, this, &AFarmPlayerController::Craft5);
}

void AFarmPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	AttackCooldown = FMath::Max(0.f, AttackCooldown - DeltaSeconds);
	UpdateTarget();
	UpdateGhost();
}

void AFarmPlayerController::UpdateTarget()
{
	CurrentTarget = nullptr;
	APawn* MyPawn = GetPawn();
	AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr;
	if (!MyPawn || !GM) return;

	const FVector PawnLocation = MyPawn->GetActorLocation();
	float BestDistance = 320.f;

	for (AFarmPlot* Plot : GM->Plots)
	{
		if (!Plot) continue;
		const float Dist = FVector::Dist2D(PawnLocation, Plot->GetActorLocation());
		if (Dist < BestDistance)
		{
			BestDistance = Dist;
			CurrentTarget = Plot;
		}
	}

	for (AFarmBuilding* Building : GM->Buildings)
	{
		if (!Building) continue;
		const float Dist = FVector::Dist2D(PawnLocation, Building->GetActorLocation());
		if (Dist < BestDistance)
		{
			BestDistance = Dist;
			CurrentTarget = Building;
		}
	}

	for (AFarmResourceNode* Node : GM->ResourceNodes)
	{
		if (!Node) continue;
		const float Dist = FVector::Dist2D(PawnLocation, Node->GetActorLocation());
		if (Dist < BestDistance)
		{
			BestDistance = Dist;
			CurrentTarget = Node;
		}
	}

	for (AFarmAnimal* Animal : GM->Animals)
	{
		if (!Animal || Animal->GetHealth() <= 0) continue;
		const float Dist = FVector::Dist2D(PawnLocation, Animal->GetActorLocation());
		if (Dist < BestDistance)
		{
			BestDistance = Dist;
			CurrentTarget = Animal;
		}
	}

	for (AFarmPortal* Portal : GM->Portals)
	{
		if (!Portal) continue;
		const float Dist = FVector::Dist2D(PawnLocation, Portal->GetActorLocation());
		if (Dist < BestDistance)
		{
			BestDistance = Dist;
			CurrentTarget = Portal;
		}
	}

	if (GM->GetBin() && FVector::Dist2D(PawnLocation, GM->GetBin()->GetActorLocation()) < BestDistance)
	{
		CurrentTarget = GM->GetBin();
	}

	if (AFarmShop* Shop = GM->GetShop())
	{
		const float Dist = FVector::Dist2D(PawnLocation, Shop->GetActorLocation());
		if (Dist < BestDistance)
		{
			CurrentTarget = Shop;
		}
	}
}

void AFarmPlayerController::UpdateGhost()
{
	if (BuildModeIndex < 0)
	{
		if (Ghost)
		{
			Ghost->Destroy();
			Ghost = nullptr;
		}
		return;
	}

	APawn* MyPawn = GetPawn();
	UWorld* World = GetWorld();
	if (!MyPawn || !World) return;

	if (!Ghost)
	{
		Ghost = World->SpawnActor<AFarmBuilding>(AFarmBuilding::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (Ghost)
		{
			Ghost->Configure(BuildModeIndex == 0 ? EBuildingType::Fence
			: BuildModeIndex == 1 ? EBuildingType::Furnace
			: EBuildingType::Chest);
			Ghost->SetActorEnableCollision(false);
			Ghost->SetActorTickEnabled(false);
		}
	}

	if (Ghost)
	{
		FVector Target = MyPawn->GetActorLocation() + MyPawn->GetActorForwardVector() * 220.f;
		Target.X = FMath::RoundToFloat(Target.X / 100.f) * 100.f;
		Target.Y = FMath::RoundToFloat(Target.Y / 100.f) * 100.f;
		Target.Z = 0.f;
		Ghost->SetActorLocation(Target);
		Ghost->SetActorHiddenInGame(false);
	}
}

FName AFarmPlayerController::GetSelectedCropId() const
{
	const AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr;
	if (!GM || GM->Crops.Num() == 0) return NAME_None;
	return GM->Crops[SelectedCropIndex % GM->Crops.Num()].Id;
}

FString AFarmPlayerController::GetSelectedCropLabel() const
{
	const AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr;
	if (!GM || GM->Crops.Num() == 0) return TEXT("none");
	return GM->Crops[SelectedCropIndex % GM->Crops.Num()].DisplayName.ToString();
}

FString AFarmPlayerController::GetPromptText() const
{
	if (IsBuildMode())
	{
		return TEXT("E: Place building    G: next type    Esc: cancel");
	}
	if (const AFarmPlot* Plot = Cast<AFarmPlot>(CurrentTarget.Get()))
	{
		return Plot->GetPrompt(this);
	}
	if (const AFarmBin* Bin = Cast<AFarmBin>(CurrentTarget.Get()))
	{
		return Bin->GetPrompt(this);
	}
	if (const AFarmBuilding* Building = Cast<AFarmBuilding>(CurrentTarget.Get()))
	{
		return Building->GetPrompt(this);
	}
	if (const AFarmResourceNode* Node = Cast<AFarmResourceNode>(CurrentTarget.Get()))
	{
		return Node->GetPrompt(this);
	}
	if (const AFarmPortal* Portal = Cast<AFarmPortal>(CurrentTarget.Get()))
	{
		return Portal->GetPrompt();
	}
	if (const AFarmAnimal* Animal = Cast<AFarmAnimal>(CurrentTarget.Get()))
	{
		return Animal->GetPrompt(this);
	}
	if (const AFarmShop* Shop = Cast<AFarmShop>(CurrentTarget.Get()))
	{
		return Shop->GetPrompt(this);
	}
	return FString();
}

void AFarmPlayerController::ShowToast(const FString& Text)
{
	ToastText = Text;
	ToastEndTime = GetWorld() ? GetWorld()->GetTimeSeconds() + 3.5f : 0.f;
}

float AFarmPlayerController::GetToastAlpha() const
{
	if (!GetWorld() || ToastText.IsEmpty()) return 0.f;
	const float Remaining = ToastEndTime - GetWorld()->GetTimeSeconds();
	return FMath::Clamp(Remaining / 1.2f, 0.f, 1.f);
}

void AFarmPlayerController::Interact()
{
	AFarmCharacter* Farmer = Cast<AFarmCharacter>(GetPawn());
	if (!Farmer) return;
	UE_LOG(LogTemp, Log, TEXT("FarmGame: interact at %s target=%s"),
		*Farmer->GetActorLocation().ToCompactString(), *GetNameSafe(CurrentTarget.Get()));

	if (IsBuildMode())
	{
		PlaceBuilding();
		return;
	}

	AActor* Target = CurrentTarget.Get();
	if (!Target) return;

	if (AFarmPlot* Plot = Cast<AFarmPlot>(Target))
	{
		Plot->Interact(Farmer);
	}
	else if (AFarmBin* Bin = Cast<AFarmBin>(Target))
	{
		Bin->Interact(Farmer);
	}
	else if (AFarmBuilding* Building = Cast<AFarmBuilding>(Target))
	{
		Building->Interact(Farmer);
	}
	else if (AFarmResourceNode* Node = Cast<AFarmResourceNode>(Target))
	{
		Node->Interact(Farmer);
	}
	else if (AFarmPortal* Portal = Cast<AFarmPortal>(Target))
	{
		Portal->Interact(Farmer);
	}
	else if (AFarmAnimal* Animal = Cast<AFarmAnimal>(Target))
	{
		Animal->Interact(Farmer);
	}
	else if (AFarmShop* Shop = Cast<AFarmShop>(Target))
	{
		Shop->Interact(Farmer);
	}
}

void AFarmPlayerController::Attack()
{
	if (AttackCooldown > 0.f) return;
	AttackCooldown = 0.45f;

	AFarmCharacter* Farmer = Cast<AFarmCharacter>(GetPawn());
	AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr;
	if (!Farmer || !GM) return;

	const FVector Origin = Farmer->GetActorLocation();
	const FVector Forward = Farmer->GetActorForwardVector().GetSafeNormal2D();
	AFarmAnimal* Best = nullptr;
	float BestDist = TNumericLimits<float>::Max();
	for (AFarmAnimal* Animal : GM->Animals)
	{
		if (!Animal || Animal->GetHealth() <= 0) continue;
		const FVector To = Animal->GetActorLocation() - Origin;
		const float Dist = To.Size2D();
		if (Dist > 320.f || Dist >= BestDist) continue;
		Best = Animal;
		BestDist = Dist;
	}

	FarmAudio::Play(this, TEXT("/Game/Art/Audio/fast_woosh.fast_woosh"), 0.8f);
	Farmer->PlayAttackAnim();
	if (Best)
	{
		Best->ApplyDamage(34, Farmer);
	}
}

void AFarmPlayerController::ToggleBuildMode()
{
	BuildModeIndex = (BuildModeIndex + 2) % 4 - 1; // -1 -> 0 -> 1 -> 2 -> -1
	if (BuildModeIndex < 0)
	{
		ExitBuildMode();
		ShowToast(TEXT("Build mode off"));
	}
	else
	{
		ShowToast(FString::Printf(TEXT("Build mode: %s (G to cycle)"), *GetBuildLabel()));
	}
}

void AFarmPlayerController::ExitBuildMode()
{
	BuildModeIndex = -1;
	if (Ghost)
	{
		Ghost->Destroy();
		Ghost = nullptr;
	}
}

FString AFarmPlayerController::GetBuildLabel() const
{
	if (BuildModeIndex == 0) return TEXT("Fence");
	if (BuildModeIndex == 1) return TEXT("Furnace");
	if (BuildModeIndex == 2) return TEXT("Chest");
	return TEXT("none");
}

void AFarmPlayerController::PlaceBuilding()
{
	AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr;
	AFarmCharacter* Farmer = Cast<AFarmCharacter>(GetPawn());
	if (!GM || !Farmer || !Ghost) return;

	const FName KitItem = BuildModeIndex == 0 ? FName(TEXT("fence_kit"))
		: BuildModeIndex == 1 ? FName(TEXT("furnace_kit"))
		: FName(TEXT("chest_kit"));
	if (Farmer->GetInventory()->GetCount(KitItem) <= 0)
	{
		ShowToast(FString::Printf(TEXT("Need %s (craft it with C)"), *FarmItems::LabelOf(KitItem)));
		return;
	}

	const FVector Location = Ghost->GetActorLocation();
	if (!GM->IsBuildLocationFree(Location, 105.f))
	{
		ShowToast(TEXT("Too close to something else"));
		return;
	}

	Farmer->GetInventory()->RemoveItem(KitItem, 1);
	const EBuildingType Type = BuildModeIndex == 0 ? EBuildingType::Fence
		: BuildModeIndex == 1 ? EBuildingType::Furnace
		: EBuildingType::Chest;
	GM->SpawnBuilding(Type, Location);
	FarmAudio::Play(this, TEXT("/Game/Art/Audio/build.build"));
	ShowToast(FString::Printf(TEXT("Placed %s"), *GetBuildLabel()));
}

void AFarmPlayerController::ToggleInventory()
{
	bInventoryOpen = !bInventoryOpen;
	if (bInventoryOpen)
	{
		bCraftMenuOpen = false;
	}

	if (InventoryWidget.IsValid())
	{
		if (bInventoryOpen)
		{
			InventoryWidget->Refresh();
			InventoryWidget->SetVisibility(EVisibility::Visible);
			bShowMouseCursor = true;
			SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(false));
		}
		else
		{
			InventoryWidget->SetVisibility(EVisibility::Collapsed);
			bShowMouseCursor = false;
			SetInputMode(FInputModeGameOnly());
		}
	}
}

void AFarmPlayerController::ToggleCraftMenu()
{
	bCraftMenuOpen = !bCraftMenuOpen;
	if (bCraftMenuOpen)
	{
		bInventoryOpen = false;
	}
}

const TArray<AFarmPlayerController::FShopEntry>& AFarmPlayerController::GetShopStock()
{
	static const TArray<FShopEntry> Stock = {
		{ TEXT("carrot_seed"), 3 }, { TEXT("potato_seed"), 5 }, { TEXT("corn_seed"), 6 },
		{ TEXT("tomato_seed"), 7 }, { TEXT("pumpkin_seed"), 10 },
		{ TEXT("hay"), 4 }, { TEXT("ore"), 8 }, { TEXT("wood"), 2 }, { TEXT("stone"), 2 },
		{ TEXT("apple"), 5 }, { TEXT("bread"), 8 }
	};
	return Stock;
}

void AFarmPlayerController::OpenShop()
{
	if (!ShopWidget.IsValid() && GEngine && GEngine->GameViewport)
	{
		SAssignNew(ShopWidget, SFarmShopWidget).PlayerController(this);
		GEngine->GameViewport->AddViewportWidgetContent(ShopWidget.ToSharedRef());
	}
	if (!ShopWidget.IsValid()) return;

	ShopWidget->Refresh();
	ShopWidget->SetVisibility(EVisibility::Visible);
	bShopOpen = true;
	bShowMouseCursor = true;
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(ShopWidget);
	SetInputMode(InputMode);
	if (AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr)
	{
		GM->SetGamePaused(true);
	}
}

void AFarmPlayerController::CloseShop()
{
	if (!ShopWidget.IsValid()) return;
	ShopWidget->SetVisibility(EVisibility::Collapsed);
	bShopOpen = false;
	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());
	if (AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr)
	{
		GM->SetGamePaused(false);
	}
}

void AFarmPlayerController::BuyFromShop(FName ItemId, int32 Price)
{
	AFarmCharacter* Farmer = Cast<AFarmCharacter>(GetPawn());
	if (!Farmer || !Farmer->GetInventory()) return;
	UFarmInventoryComponent* Inventory = Farmer->GetInventory();
	if (!Inventory->SpendGold(Price))
	{
		ShowToast(TEXT("Not enough gold"));
		return;
	}
	Inventory->AddItem(ItemId, 1);
	FarmAudio::Play(this, TEXT("/Game/Art/Audio/craft.craft"), 0.7f);
	ShowToast(FString::Printf(TEXT("Bought %s"), *FarmItems::LabelOf(ItemId)));
	if (ShopWidget.IsValid()) ShopWidget->Refresh();
}

void AFarmPlayerController::EatFood()
{
	AFarmCharacter* Farmer = Cast<AFarmCharacter>(GetPawn());
	if (!Farmer || !Farmer->GetInventory()) return;
	UFarmInventoryComponent* Inventory = Farmer->GetInventory();

	struct FFood { FName Id; int32 Heal; };
	const FFood Foods[] = { { TEXT("bread"), 20 }, { TEXT("apple"), 10 }, { TEXT("meat"), 15 }, { TEXT("milk"), 8 }, { TEXT("egg"), 5 } };
	for (const FFood& Food : Foods)
	{
		if (Inventory->GetCount(Food.Id) > 0)
		{
			Inventory->RemoveItem(Food.Id, 1);
			Farmer->Heal(Food.Heal);
			FarmAudio::Play(this, TEXT("/Game/Art/Audio/pop.pop"), 0.8f);
			ShowToast(FString::Printf(TEXT("Ate %s (+%d HP)"), *FarmItems::LabelOf(Food.Id), Food.Heal));
			return;
		}
	}
	ShowToast(TEXT("No food in the bag"));
}

void AFarmPlayerController::ShowMenu(bool bStartMode)
{
	if (!MenuWidget.IsValid()) return;
	bMenuStartMode = bStartMode;
	MenuWidget->SetStartMode(bStartMode);
	MenuWidget->SetVisibility(EVisibility::Visible);
	bShowMouseCursor = true;
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(MenuWidget);
	SetInputMode(InputMode);
	if (AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr)
	{
		GM->SetGamePaused(true);
	}
}

void AFarmPlayerController::HideMenu()
{
	if (!MenuWidget.IsValid()) return;
	MenuWidget->SetVisibility(EVisibility::Collapsed);
	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());
	if (AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr)
	{
		GM->SetGamePaused(false);
	}
}

void AFarmPlayerController::HandleMenuPrimary()
{
	if (bMenuStartMode)
	{
		if (AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr)
		{
			GM->StartGame();
		}
		HideMenu();
	}
	else
	{
		HideMenu();
	}
}

void AFarmPlayerController::HandleMenuEscape()
{
	if (!bMenuStartMode)
	{
		HideMenu();
	}
}

void AFarmPlayerController::HandleMenuQuit()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

void AFarmPlayerController::CancelMode()
{
	if (bShopOpen)
	{
		CloseShop();
		return;
	}
	if (MenuWidget.IsValid() && MenuWidget->GetVisibility() == EVisibility::Visible)
	{
		HandleMenuEscape();
		return;
	}
	if (!bInventoryOpen && !bCraftMenuOpen && BuildModeIndex < 0)
	{
		ShowMenu(false);
		return;
	}
	if (bInventoryOpen)
	{
		ToggleInventory();
		return;
	}
	if (bCraftMenuOpen)
	{
		bCraftMenuOpen = false;
		return;
	}
	if (IsBuildMode())
	{
		ExitBuildMode();
	}
}

bool AFarmPlayerController::IsNearFurnace() const
{
	APawn* MyPawn = GetPawn();
	AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr;
	if (!MyPawn || !GM) return false;
	for (const AFarmBuilding* Building : GM->Buildings)
	{
		if (Building && Building->GetBuildingType() == EBuildingType::Furnace &&
			FVector::Dist2D(MyPawn->GetActorLocation(), Building->GetActorLocation()) < 320.f)
		{
			return true;
		}
	}
	return false;
}

bool AFarmPlayerController::CanCraftRecipe(int32 Index) const
{
	AFarmCharacter* Farmer = Cast<AFarmCharacter>(GetPawn());
	const TArray<FCraftRecipe>& Recipes = FarmRecipes::All();
	if (!Farmer || !Recipes.IsValidIndex(Index)) return false;
	if (Recipes[Index].bNeedsFurnace && !IsNearFurnace()) return false;
	return Farmer->GetInventory()->CanCraft(Recipes[Index]);
}

void AFarmPlayerController::CraftRecipe(int32 Index)
{
	AFarmCharacter* Farmer = Cast<AFarmCharacter>(GetPawn());
	const TArray<FCraftRecipe>& Recipes = FarmRecipes::All();
	if (!Farmer || !Recipes.IsValidIndex(Index)) return;

	const FCraftRecipe& Recipe = Recipes[Index];
	if (Recipe.bNeedsFurnace && !IsNearFurnace())
	{
		ShowToast(TEXT("Stand next to a furnace to smelt"));
		return;
	}
	if (Farmer->GetInventory()->Craft(Recipe))
	{
		FarmAudio::Play(this, TEXT("/Game/Art/Audio/craft.craft"));
		if (AFarmGameMode* Gm = GetWorld()->GetAuthGameMode<AFarmGameMode>()) Gm->NotifyProduced(Recipe.OutputId, Recipe.OutputCount);
		UE_LOG(LogTemp, Log, TEXT("FarmGame: crafted %s"), *Recipe.OutputId.ToString());
		ShowToast(FString::Printf(TEXT("Crafted %s"), *FarmItems::LabelOf(Recipe.OutputId)));
	}
	else
	{
		ShowToast(TEXT("Missing ingredients"));
	}
}

void AFarmPlayerController::Craft1() { CraftRecipe(0); }
void AFarmPlayerController::Craft2() { CraftRecipe(1); }
void AFarmPlayerController::Craft3() { CraftRecipe(2); }
void AFarmPlayerController::Craft4() { CraftRecipe(3); }
void AFarmPlayerController::Craft5() { CraftRecipe(4); }

void AFarmPlayerController::CycleSeed()
{
	AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr;
	if (!GM || GM->Crops.Num() == 0) return;
	SelectedCropIndex = (SelectedCropIndex + 1) % GM->Crops.Num();
	ShowToast(FString::Printf(TEXT("Selected seed: %s"), *GetSelectedCropLabel()));
}

void AFarmPlayerController::BuySeed()
{
	AFarmCharacter* Farmer = Cast<AFarmCharacter>(GetPawn());
	AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr;
	if (!Farmer || !GM || !CurrentTarget.IsValid() || !Cast<AFarmBin>(CurrentTarget.Get())) return;

	const FCropDef* Crop = GM->FindCrop(GetSelectedCropId());
	if (!Crop) return;
	if (Farmer->GetInventory()->BuyItem(FarmItems::SeedId(Crop->Id), 1))
	{
		ShowToast(FString::Printf(TEXT("Bought 1 %s seed for %d gold"), *Crop->DisplayName.ToString(), Crop->SeedCost));
	}
	else
	{
		ShowToast(TEXT("Not enough gold"));
	}
}

void AFarmPlayerController::SaveGame()
{
	AFarmCharacter* Farmer = Cast<AFarmCharacter>(GetPawn());
	AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr;
	if (Farmer && GM)
	{
		GM->SaveGame(Farmer);
		ShowToast(TEXT("Game saved (Ctrl+L to load)"));
	}
}

void AFarmPlayerController::LoadGame()
{
	AFarmCharacter* Farmer = Cast<AFarmCharacter>(GetPawn());
	AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr;
	if (Farmer && GM)
	{
		ShowToast(GM->LoadGame(Farmer) ? TEXT("Game loaded") : TEXT("No save found"));
	}
}

void AFarmPlayerController::Sleep()
{
	AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr;
	if (!GM) return;
	const int32 Before = GM->GetDay();
	GM->AdvanceDay();
	ShowToast(FString::Printf(TEXT("You sleep... day %d -> %d"), Before, GM->GetDay()));
}
