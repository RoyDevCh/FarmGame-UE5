#include "FarmInventoryComponent.h"

UFarmInventoryComponent::UFarmInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFarmInventoryComponent::ResetForNewGame()
{
	Gold = 20;
	Items.Reset();
	Items.Add(TEXT("carrot_seed"), 3);
	Items.Add(TEXT("corn_seed"), 1);
	Items.Add(TEXT("wood"), 6);
	Items.Add(TEXT("hay"), 1);
}

int32 UFarmInventoryComponent::GetCount(FName ItemId) const
{
	const int32* Count = Items.Find(ItemId);
	return Count ? *Count : 0;
}

void UFarmInventoryComponent::AddItem(FName ItemId, int32 Amount)
{
	if (ItemId.IsNone() || Amount <= 0) return;
	Items.FindOrAdd(ItemId) += Amount;
}

bool UFarmInventoryComponent::RemoveItem(FName ItemId, int32 Amount)
{
	if (Amount <= 0) return true;
	int32* Count = Items.Find(ItemId);
	if (!Count || *Count < Amount) return false;
	*Count -= Amount;
	if (*Count <= 0) Items.Remove(ItemId);
	return true;
}

bool UFarmInventoryComponent::SpendGold(int32 Cost)
{
	if (Cost < 0 || Gold < Cost) return false;
	Gold -= Cost;
	return true;
}

void UFarmInventoryComponent::AddGold(int32 Amount)
{
	if (Amount > 0) Gold += Amount;
}

bool UFarmInventoryComponent::BuyItem(FName ItemId, int32 Amount)
{
	const FItemDef* Def = FarmItems::Find(ItemId);
	if (!Def || Def->BuyPrice <= 0 || Amount <= 0) return false;
	const int32 Total = Def->BuyPrice * Amount;
	if (!SpendGold(Total)) return false;
	AddItem(ItemId, Amount);
	return true;
}

int32 UFarmInventoryComponent::SellEverything()
{
	int32 Earned = 0;
	TArray<FName> Sold;
	for (const TPair<FName, int32>& Pair : Items)
	{
		const FItemDef* Def = FarmItems::Find(Pair.Key);
		if (!Def || Pair.Value <= 0) continue;
		// Seeds and raw resources stay in the bag; produce, animal goods and ingots sell.
		if (Def->Kind == EItemKind::Seed || Def->Kind == EItemKind::Resource) continue;
		if (Def->SellPrice > 0)
		{
			Earned += Def->SellPrice * Pair.Value;
			Sold.Add(Pair.Key);
		}
	}
	for (const FName& Id : Sold)
	{
		Items.Remove(Id);
	}
	AddGold(Earned);
	return Earned;
}

bool UFarmInventoryComponent::CanCraft(const FCraftRecipe& Recipe) const
{
	for (const FCraftIngredient& Ingredient : Recipe.Inputs)
	{
		if (GetCount(Ingredient.ItemId) < Ingredient.Count) return false;
	}
	return true;
}

bool UFarmInventoryComponent::Craft(const FCraftRecipe& Recipe)
{
	if (!CanCraft(Recipe)) return false;
	for (const FCraftIngredient& Ingredient : Recipe.Inputs)
	{
		RemoveItem(Ingredient.ItemId, Ingredient.Count);
	}
	AddItem(Recipe.OutputId, Recipe.OutputCount);
	return true;
}

int32 UFarmInventoryComponent::GetTotalItemCount() const
{
	int32 Total = 0;
	for (const TPair<FName, int32>& Pair : Items) Total += Pair.Value;
	return Total;
}

int32 UFarmInventoryComponent::GetSellableCount() const
{
	int32 Total = 0;
	for (const TPair<FName, int32>& Pair : Items)
	{
		const FItemDef* Def = FarmItems::Find(Pair.Key);
		if (Def && Def->SellPrice > 0 && Def->Kind != EItemKind::Seed && Def->Kind != EItemKind::Resource)
		{
			Total += Pair.Value;
		}
	}
	return Total;
}
