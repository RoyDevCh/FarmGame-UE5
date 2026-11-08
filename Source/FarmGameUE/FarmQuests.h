#pragma once

#include "CoreMinimal.h"
#include "FarmQuests.generated.h"

/** One objective in the linear tutorial chain. */
USTRUCT()
struct FQuestState
{
	GENERATED_BODY()

	UPROPERTY() FString Title;
	UPROPERTY() FName ItemId;
	UPROPERTY() int32 Target = 1;
	UPROPERTY() int32 Progress = 0;
	/** Gold granted on completion. */
	UPROPERTY() int32 Reward = 0;
};

namespace FarmQuests
{
	inline TArray<FQuestState> MakeDefault()
	{
		TArray<FQuestState> Quests;

		FQuestState Harvest;
		Harvest.Title = TEXT("Harvest 3 carrots");
		Harvest.ItemId = TEXT("carrot");
		Harvest.Target = 3;
		Harvest.Reward = 20;
		Quests.Add(Harvest);

		FQuestState Craft;
		Craft.Title = TEXT("Craft a furnace kit");
		Craft.ItemId = TEXT("furnace_kit");
		Craft.Target = 1;
		Craft.Reward = 30;
		Quests.Add(Craft);

		FQuestState Smelt;
		Smelt.Title = TEXT("Smelt an iron ingot");
		Smelt.ItemId = TEXT("ingot");
		Smelt.Target = 1;
		Smelt.Reward = 40;
		Quests.Add(Smelt);

		FQuestState Axe;
		Axe.Title = TEXT("Craft an axe");
		Axe.ItemId = TEXT("axe");
		Axe.Target = 1;
		Axe.Reward = 50;
		Quests.Add(Axe);

		FQuestState Corn;
		Corn.Title = TEXT("Harvest 5 corn");
		Corn.ItemId = TEXT("corn");
		Corn.Target = 5;
		Corn.Reward = 60;
		Quests.Add(Corn);

		FQuestState Steel;
		Steel.Title = TEXT("Smelt 3 iron ingots");
		Steel.ItemId = TEXT("ingot");
		Steel.Target = 3;
		Steel.Reward = 80;
		Quests.Add(Steel);

		FQuestState Shop;
		Shop.Title = TEXT("Buy an apple at the shop");
		Shop.ItemId = TEXT("apple");
		Shop.Target = 1;
		Shop.Reward = 30;
		Quests.Add(Shop);

		FQuestState SteelAxe;
		SteelAxe.Title = TEXT("Craft a steel axe");
		SteelAxe.ItemId = TEXT("steel_axe");
		SteelAxe.Target = 1;
		SteelAxe.Reward = 100;
		Quests.Add(SteelAxe);

		return Quests;
	}

	/** Adds progress; returns true when the quest just completed. */
	inline bool Advance(FQuestState& Quest, int32 Amount)
	{
		if (Amount <= 0 || Quest.Progress >= Quest.Target) return false;
		Quest.Progress = FMath::Min(Quest.Target, Quest.Progress + Amount);
		return Quest.Progress >= Quest.Target;
	}
}
