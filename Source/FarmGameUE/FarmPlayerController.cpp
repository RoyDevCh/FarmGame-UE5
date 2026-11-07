#include "FarmPlayerController.h"
#include "FarmBin.h"
#include "FarmCharacter.h"
#include "FarmGameMode.h"
#include "FarmInventoryComponent.h"
#include "FarmPlot.h"
#include "EngineUtils.h"

AFarmPlayerController::AFarmPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFarmPlayerController::BeginPlay()
{
	Super::BeginPlay();
	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());
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
}

void AFarmPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateTarget();
}

void AFarmPlayerController::UpdateTarget()
{
	CurrentTarget = nullptr;
	APawn* MyPawn = GetPawn();
	AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr;
	if (!MyPawn || !GM) return;

	const FVector PawnLocation = MyPawn->GetActorLocation();
	float BestDistance = 260.f;

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

	if (GM->GetBin() && FVector::Dist2D(PawnLocation, GM->GetBin()->GetActorLocation()) < BestDistance)
	{
		CurrentTarget = GM->GetBin();
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
	if (const AFarmPlot* Plot = Cast<AFarmPlot>(CurrentTarget.Get()))
	{
		return Plot->GetPrompt(this);
	}
	if (const AFarmBin* Bin = Cast<AFarmBin>(CurrentTarget.Get()))
	{
		return Bin->GetPrompt(this);
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
	AActor* Target = CurrentTarget.Get();
	AFarmCharacter* Farmer = Cast<AFarmCharacter>(GetPawn());
	if (!Target || !Farmer) return;

	if (AFarmPlot* Plot = Cast<AFarmPlot>(Target))
	{
		Plot->Interact(Farmer);
	}
	else if (AFarmBin* Bin = Cast<AFarmBin>(Target))
	{
		Bin->Interact(Farmer);
	}
}

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
	if (Farmer->GetInventory()->BuySeed(Crop->Id, Crop->SeedCost))
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
		GM->SaveGame(Farmer->GetInventory());
		ShowToast(TEXT("Game saved (F9 to load)"));
	}
}

void AFarmPlayerController::LoadGame()
{
	AFarmCharacter* Farmer = Cast<AFarmCharacter>(GetPawn());
	AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr;
	if (Farmer && GM)
	{
		ShowToast(GM->LoadGame(Farmer->GetInventory()) ? TEXT("Game loaded") : TEXT("No save found"));
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