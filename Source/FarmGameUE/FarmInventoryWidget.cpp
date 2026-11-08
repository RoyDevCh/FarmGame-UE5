#include "FarmInventoryWidget.h"
#include "FarmCharacter.h"
#include "FarmGameMode.h"
#include "FarmInventoryComponent.h"
#include "FarmItems.h"
#include "FarmPlayerController.h"
#include "FarmRecipes.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "FarmInventory"

void SFarmInventoryWidget::Construct(const FArguments& InArgs)
{
	PlayerController = InArgs._PlayerController;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(FLinearColor(0.05f, 0.06f, 0.05f, 0.96f))
		.Padding(18.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Title", "INVENTORY & CRAFTING  -  Tab closes, click a recipe or press 1-5"))
				.ColorAndOpacity(FLinearColor(1.f, 0.9f, 0.55f))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 15))
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SAssignNew(ItemList, SVerticalBox)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CraftTitle", "Crafting"))
				.ColorAndOpacity(FLinearColor(0.7f, 0.9f, 0.7f))
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SAssignNew(CraftList, SVerticalBox)
			]
		]
	];

	Refresh();
}

void SFarmInventoryWidget::Refresh()
{
	if (!ItemList.IsValid() || !CraftList.IsValid()) return;

	ItemList->ClearChildren();
	CraftList->ClearChildren();

	AFarmPlayerController* PC = PlayerController.Get();
	AFarmCharacter* Character = PC ? Cast<AFarmCharacter>(PC->GetPawn()) : nullptr;
	UFarmInventoryComponent* Inventory = Character ? Character->GetInventory() : nullptr;
	if (!Inventory) return;

	struct FGroupDef
	{
		EItemKind Kind;
		const TCHAR* Title;
	};
	const FGroupDef Groups[] = {
		{ EItemKind::Seed, TEXT("Seeds") },
		{ EItemKind::Produce, TEXT("Harvest") },
		{ EItemKind::AnimalProduct, TEXT("Animal goods") },
		{ EItemKind::Resource, TEXT("Resources") },
		{ EItemKind::Material, TEXT("Materials / kits") },
		{ EItemKind::Food, TEXT("Food") }
	};

	for (const FGroupDef& Group : Groups)
	{
		bool bAny = false;
		for (const FItemDef& Def : FarmItems::All())
		{
			if (Def.Kind == Group.Kind && Inventory->GetCount(Def.Id) > 0) { bAny = true; break; }
		}
		if (!bAny) continue;

		ItemList->AddSlot().AutoHeight().Padding(0, 6, 0, 2)
		[
			SNew(STextBlock).Text(FText::FromString(Group.Title)).ColorAndOpacity(FLinearColor(0.65f, 0.85f, 0.65f))
		];

		for (const FItemDef& Def : FarmItems::All())
		{
			if (Def.Kind != Group.Kind) continue;
			const int32 Count = Inventory->GetCount(Def.Id);
			if (Count <= 0) continue;

			const FText Label = Def.SellPrice > 0
				? FText::FromString(FString::Printf(TEXT("  %s  x%d      sell %dg"), Def.Label, Count, Def.SellPrice))
				: FText::FromString(FString::Printf(TEXT("  %s  x%d"), Def.Label, Count));
			ItemList->AddSlot().AutoHeight()
			[
				SNew(STextBlock).Text(Label).ColorAndOpacity(FLinearColor(0.9f, 0.92f, 0.88f))
			];
		}
	}

	const TArray<FCraftRecipe>& Recipes = FarmRecipes::All();
	for (int32 i = 0; i < Recipes.Num(); ++i)
	{
		const FCraftRecipe& Recipe = Recipes[i];
		FString Ingredients;
		for (const FCraftIngredient& Ingredient : Recipe.Inputs)
		{
			Ingredients += FString::Printf(TEXT("%s x%d  "), *FarmItems::LabelOf(Ingredient.ItemId), Ingredient.Count);
		}
		const bool bEnabled = PC->CanCraftRecipe(i);
		CraftList->AddSlot().AutoHeight().Padding(0, 2)
		[
			SNew(SButton)
			.IsEnabled(bEnabled)
			.OnClicked_Lambda([PC, i]() { PC->CraftRecipe(i); return FReply::Handled(); })
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%s   (%s)%s"), Recipe.Label, *Ingredients,
					Recipe.bNeedsFurnace ? TEXT("  [needs furnace]") : TEXT(""))))
			]
		];
	}
}

#undef LOCTEXT_NAMESPACE
