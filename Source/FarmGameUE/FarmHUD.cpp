#include "FarmHUD.h"
#include "FarmBuilding.h"
#include "FarmCharacter.h"
#include "FarmGameMode.h"
#include "FarmInventoryComponent.h"
#include "FarmItems.h"
#include "FarmPlayerController.h"
#include "FarmRecipes.h"
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

	const FString Status = FString::Printf(TEXT("%s   %s"), *GM->GetTimeLabel(), *GM->GetWeatherLabel());
	float StatusW = 0.f, StatusH = 0.f;
	GetTextSize(Status, StatusW, StatusH, Small, 1.2f);
	DrawText(Status, FLinearColor(0.85f, 0.92f, 1.f), W - Margin - StatusW, Margin + 4.f, Small, 1.2f);

	const float HealthPercent = Character->GetMaxHealth() > 0 ? static_cast<float>(Character->GetHealth()) / Character->GetMaxHealth() : 0.f;
	DrawRect(FLinearColor(0.25f, 0.05f, 0.05f), Margin, H - Margin - 58.f, 240.f, 16.f);
	DrawRect(FLinearColor(0.25f, 0.85f, 0.35f), Margin, H - Margin - 58.f, 240.f * HealthPercent, 16.f);
	DrawText(FString::Printf(TEXT("HP %d / %d"), Character->GetHealth(), Character->GetMaxHealth()),
		FLinearColor(0.95f, 0.95f, 0.95f), Margin, H - Margin - 84.f, Small, 1.05f);

	float Y = Margin + 48.f;
	DrawText(FString::Printf(TEXT("Selected seed: %s  (Q to change)"), *PC->GetSelectedCropLabel()), FLinearColor(0.85f, 0.95f, 1.f), Margin, Y, Small, 1.15f);
	Y += 26.f;

	for (const FCropDef& Crop : GM->Crops)
	{
		const int32 Seeds = Character->GetInventory()->GetCount(FarmItems::SeedId(Crop.Id));
		const int32 Produce = Character->GetInventory()->GetCount(Crop.Id);
		const FString Line = FString::Printf(TEXT("%s: %d seeds, %d harvested  (seed %dg / sell %dg)"),
			*Crop.DisplayName.ToString(), Seeds, Produce, Crop.SeedCost, Crop.SellPrice);
		DrawText(Line, FLinearColor(0.82f, 0.86f, 0.8f), Margin, Y, Small, 1.05f);
		Y += 22.f;
	}

	Y += 6.f;
	DrawText(FString::Printf(TEXT("Wood %d   Stone %d   Ore %d   Ingot %d   Hay %d"),
		Character->GetInventory()->GetCount(TEXT("wood")),
		Character->GetInventory()->GetCount(TEXT("stone")),
		Character->GetInventory()->GetCount(TEXT("ore")),
		Character->GetInventory()->GetCount(TEXT("ingot")),
		Character->GetInventory()->GetCount(TEXT("hay"))),
		FLinearColor(0.72f, 0.78f, 0.7f), Margin, Y, Small, 1.0f);
	Y += 22.f;
	DrawText(FString::Printf(TEXT("Kits: fence %d   furnace %d"),
		Character->GetInventory()->GetCount(TEXT("fence_kit")),
		Character->GetInventory()->GetCount(TEXT("furnace_kit"))),
		FLinearColor(0.72f, 0.78f, 0.7f), Margin, Y, Small, 1.0f);

	const FString Help = TEXT("WASD move | E interact | F attack | H eat | Q seed | B buy | C craft | G build | Tab bag | T sleep | Esc pause | Ctrl+S/L");
	DrawText(Help, FLinearColor(0.55f, 0.6f, 0.65f), Margin, H - Margin - 22.f, Small, 1.0f);

	if (PC->IsBuildMode())
	{
		const FString BuildLine = FString::Printf(TEXT("BUILD MODE: %s   (G cycle, E place, Esc cancel)"), *PC->GetBuildLabel());
		float BW = 0.f, BH = 0.f;
		GetTextSize(BuildLine, BW, BH, Small, 1.15f);
		DrawText(BuildLine, FLinearColor(0.6f, 1.f, 0.6f), (W - BW) * 0.5f, H - 210.f, Small, 1.15f);
	}

	const FString Prompt = PC->GetPromptText();
	if (!Prompt.IsEmpty() && !PC->IsCraftMenuOpen())
	{
		float TextW = 0.f, TextH = 0.f;
		GetTextSize(Prompt, TextW, TextH, Small, 1.2f);
		DrawText(Prompt, FLinearColor(1.f, 1.f, 1.f), (W - TextW) * 0.5f, H - 140.f, Small, 1.2f);
	}

	if (PC->IsCraftMenuOpen())
	{
		DrawText(TEXT("CRAFTING  (C or Esc to close)"), FLinearColor(1.f, 0.9f, 0.5f), Margin, H - 420.f, Medium, 1.15f);
		const TArray<FCraftRecipe>& Recipes = FarmRecipes::All();
		float RY = H - 386.f;
		for (int32 i = 0; i < Recipes.Num(); ++i)
		{
			const FCraftRecipe& Recipe = Recipes[i];
			FString Ingredients;
			for (const FCraftIngredient& Ingredient : Recipe.Inputs)
			{
				Ingredients += FString::Printf(TEXT("%s x%d  "), *FarmItems::LabelOf(Ingredient.ItemId), Ingredient.Count);
			}
			const bool bCanCraft = PC->CanCraftRecipe(i);
			const bool bFurnaceOnly = Recipe.bNeedsFurnace;
			const FString Line = FString::Printf(TEXT("[%d] %s   %s%s"), i + 1, Recipe.Label, *Ingredients,
				bFurnaceOnly ? TEXT("(needs furnace)") : TEXT(""));
			DrawText(Line, bCanCraft ? FLinearColor(0.55f, 1.f, 0.55f) : FLinearColor(0.55f, 0.55f, 0.55f), Margin, RY, Small, 1.05f);
			RY += 24.f;
		}
	}

	if (PC->IsInventoryOpen() && !PC->IsSlateInventoryActive())
	{
		const float PX = W * 0.18f;
		const float PY = H * 0.10f;
		const float PW = W * 0.64f;
		const float PH = H * 0.76f;
		DrawRect(FLinearColor(0.02f, 0.03f, 0.02f, 0.88f), PX, PY, PW, PH);
		DrawText(TEXT("INVENTORY   (Tab or Esc to close)"), FLinearColor(1.f, 0.9f, 0.55f), PX + 18.f, PY + 14.f, Medium, 1.2f);

		struct FInventoryGroup
		{
			EItemKind Kind;
			const TCHAR* Title;
		};
		const FInventoryGroup Groups[] = {
			{ EItemKind::Seed, TEXT("Seeds") },
			{ EItemKind::Produce, TEXT("Harvest") },
			{ EItemKind::AnimalProduct, TEXT("Animal goods") },
			{ EItemKind::Resource, TEXT("Resources") },
			{ EItemKind::Material, TEXT("Materials / kits") }
		};

		float ColY = PY + 56.f;
		for (const FInventoryGroup& Group : Groups)
		{
			DrawText(Group.Title, FLinearColor(0.65f, 0.85f, 0.65f), PX + 18.f, ColY, Small, 1.1f);
			ColY += 24.f;
			for (const FItemDef& Def : FarmItems::All())
			{
				if (Def.Kind != Group.Kind) continue;
				const int32 Count = Character->GetInventory()->GetCount(Def.Id);
				if (Count <= 0) continue;
				const int32 UnitPrice = Def.SellPrice;
				const FString Line = UnitPrice > 0
					? FString::Printf(TEXT("  %-16s x%-3d   sell %dg"), Def.Label, Count, UnitPrice)
					: FString::Printf(TEXT("  %-16s x%d"), Def.Label, Count);
				DrawText(Line, FLinearColor(0.88f, 0.9f, 0.86f), PX + 24.f, ColY, Small, 1.0f);
				ColY += 20.f;
			}
			ColY += 8.f;
		}

		DrawText(FString::Printf(TEXT("Gold %d      HP %d/%d"), Character->GetInventory()->Gold, Character->GetHealth(), Character->GetMaxHealth()),
			FLinearColor(1.f, 0.92f, 0.7f), PX + 18.f, PY + PH - 34.f, Small, 1.1f);
	}

	if (const AFarmGameMode* QuestGM = PC->GetWorld() ? PC->GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr)
	{
		const TArray<FQuestState>& Quests = QuestGM->GetQuests();
		const int32 Index = QuestGM->GetActiveQuestIndex();
		FString Line = TEXT("Objective: all complete");
		if (Quests.IsValidIndex(Index))
		{
			Line = FString::Printf(TEXT("Objective: %s  (%d/%d)"), *Quests[Index].Title, Quests[Index].Progress, Quests[Index].Target);
		}
		DrawText(Line, FLinearColor(1.f, 0.85f, 0.45f), W * 0.02f, H * 0.34f, Small, 1.05f);
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
