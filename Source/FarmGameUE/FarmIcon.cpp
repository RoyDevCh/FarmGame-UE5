#include "FarmIcon.h"
#include "Engine/Texture2D.h"
#include "UObject/StrongObjectPtr.h"

namespace
{
	TMap<FName, TSharedPtr<FSlateBrush>> GIconBrushes;
	TArray<TStrongObjectPtr<UTexture2D>> GIconTextures;

	FString IconNameFor(FName ItemId)
	{
		static const TMap<FName, FString> Overrides = {
			{ TEXT("stone"), TEXT("StoneTiles") },
			{ TEXT("hay"), TEXT("hay_item") },
			{ TEXT("milk"), TEXT("Milk_Container") },
			{ TEXT("steel_axe"), TEXT("Axe") },
			{ TEXT("steel_pickaxe"), TEXT("Pickaxe") },
			{ TEXT("cooked_fish"), TEXT("Fish") }
		};
		if (const FString* Found = Overrides.Find(ItemId)) return *Found;

		FString Name = ItemId.ToString();
		if (!Name.IsEmpty())
		{
			Name[0] = FChar::ToUpper(Name[0]);
		}
		return Name;
	}
}

TSharedPtr<FSlateBrush> FarmIcon::BrushFor(FName ItemId)
{
	if (ItemId.IsNone()) return nullptr;
	if (const TSharedPtr<FSlateBrush>* Found = GIconBrushes.Find(ItemId)) return *Found;

	const FString Name = IconNameFor(ItemId);
	UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, *FString::Printf(TEXT("/Game/Art/Icons/%s.%s"), *Name, *Name));
	if (!Texture)
	{
		GIconBrushes.Add(ItemId, nullptr);
		return nullptr;
	}

	GIconTextures.Emplace(Texture);
	TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
	Brush->SetResourceObject(Texture);
	Brush->ImageSize = FVector2D(24.f, 24.f);
	GIconBrushes.Add(ItemId, Brush);
	return Brush;
}
