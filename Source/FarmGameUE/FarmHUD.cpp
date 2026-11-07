#include "FarmHUD.h"
#include "FarmCharacter.h"
#include "FarmGameMode.h"
#include "FarmInventoryComponent.h"
#include "FarmPlayerController.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

void AFarmHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas) return;

	AFarmPlayerController* PC = Cast<AFarmPlayerController>(PlayerOwner);
	AFarmCharacter* Character = PC ? Cast<AFarmCharacter>(PC->GetPawn()) : nullptr;
	AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr;
	if (!PC || !Character || !GM) return;

	UFont* Small = GEngine->GetSmallFont();
	UFont* Medium = GEngine->GetMediumFont();
	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;
	const float Margin = 28.f;

	const FString Header = FString::Printf(TEXT("Day %d      Gold %d g"), GM->GetDay(), Character->GetInventory()->Gold);
	DrawText(Header, FLinearColor(1.f, 0.94f, 0.75f), Margin, Margin, Medium, 1.25f);

	float Y = Margin + 48.f;
	DrawText(FString::Printf(TEXT("Selected seed: %s  (Q to change)"), *PC->GetSelectedCropLabel()), FLinearColor(0.85f, 0.95f, 1.f), Margin, Y, Small, 1.15f);
	Y += 26.f;

	for (const FCropDef& Crop : GM->Crops)
	{
		const int32 Seeds = Character->GetInventory()->GetSeedCount(Crop.Id);
		const int32 Produce = Character->GetInventory()->GetProduceCount(Crop.Id);
		const FString Line = FString::Printf(TEXT("%s: %d seeds, %d harvested  (seed %dg / sell %dg)"),
			*Crop.DisplayName.ToString(), Seeds, Produce, Crop.SeedCost, Crop.SellPrice);
		DrawText(Line, FLinearColor(0.82f, 0.86f, 0.8f), Margin, Y, Small, 1.05f);
		Y += 22.f;
	}

	const FString Help = TEXT("WASD move | E interact | Q seed | B buy at bin | T sleep | Ctrl+S save | Ctrl+L load");
	DrawText(Help, FLinearColor(0.55f, 0.6f, 0.65f), Margin, H - Margin - 22.f, Small, 1.0f);

	const FString Prompt = PC->GetPromptText();
	if (!Prompt.IsEmpty())
	{
		float TextW = 0.f, TextH = 0.f;
		GetTextSize(Prompt, TextW, TextH, Small, 1.2f);
		DrawText(Prompt, FLinearColor(1.f, 1.f, 1.f), (W - TextW) * 0.5f, H - 140.f, Small, 1.2f);
	}

	const float ToastAlpha = PC->GetToastAlpha();
	if (ToastAlpha > 0.f)
	{
		const FString& Toast = PC->GetToastText();
		float TextW = 0.f, TextH = 0.f;
		GetTextSize(Toast, TextW, TextH, Small, 1.25f);
		DrawText(Toast, FLinearColor(1.f, 0.9f, 0.4f, ToastAlpha), (W - TextW) * 0.5f, 120.f, Small, 1.25f);
	}
}
