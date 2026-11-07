#include "FarmInventoryComponent.h"

UFarmInventoryComponent::UFarmInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFarmInventoryComponent::ResetForNewGame()
{
	Gold = 20;
	Seeds.Reset();
	Produce.Reset();
	Seeds.Add(TEXT("carrot"), 3);
}

bool UFarmInventoryComponent::BuySeed(FName CropId, int32 SeedCost, int32 Amount)
{
	if (CropId.IsNone() || Amount <= 0) return false;
	const int32 Total = FMath::Max(0, SeedCost) * Amount;
	if (Gold < Total) return false;
	Gold -= Total;
	AddSeed(CropId, Amount);
	return true;
}

bool UFarmInventoryComponent::ConsumeSeed(FName CropId, int32 Amount)
{
	int32* Count = Seeds.Find(CropId);
	if (!Count || *Count < Amount) return false;
	*Count -= Amount;
	if (*Count <= 0) Seeds.Remove(CropId);
	return true;
}

void UFarmInventoryComponent::AddSeed(FName CropId, int32 Amount)
{
	if (CropId.IsNone() || Amount <= 0) return;
	Seeds.FindOrAdd(CropId) += Amount;
}

void UFarmInventoryComponent::AddProduce(FName CropId, int32 Amount)
{
	if (CropId.IsNone() || Amount <= 0) return;
	Produce.FindOrAdd(CropId) += Amount;
}

int32 UFarmInventoryComponent::GetSeedCount(FName CropId) const
{
	const int32* Count = Seeds.Find(CropId);
	return Count ? *Count : 0;
}

int32 UFarmInventoryComponent::GetProduceCount(FName CropId) const
{
	const int32* Count = Produce.Find(CropId);
	return Count ? *Count : 0;
}

int32 UFarmInventoryComponent::SellAllProduce(const TArray<FCropDef>& Crops)
{
	int32 Earned = 0;
	for (const TPair<FName, int32>& Pair : Produce)
	{
		const FCropDef* Def = Crops.FindByPredicate([&Pair](const FCropDef& C) { return C.Id == Pair.Key; });
		if (Def)
		{
			Earned += Def->SellPrice * Pair.Value;
		}
	}
	Gold += Earned;
	Produce.Reset();
	return Earned;
}

int32 UFarmInventoryComponent::GetTotalProduce() const
{
	int32 Total = 0;
	for (const TPair<FName, int32>& Pair : Produce) Total += Pair.Value;
	return Total;
}
