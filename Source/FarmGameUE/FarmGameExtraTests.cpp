#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "FarmTypes.h"
#include "FarmInventoryComponent.h"
#include "FarmItems.h"
#include "FarmRecipes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Walks a two-days-per-stage crop through every stage with water every day.
 * Covers stage 0 -> 1 -> 2 -> 3, the growth-day bookkeeping and the mature stop.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmExtraCropMultiStageTest, "FarmGame.Extra.CropMultiStageContinuous",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmExtraCropMultiStageTest::RunTest(const FString& Parameters)
{
	FCropDef Wheat;
	Wheat.Id = TEXT("wheat");
	Wheat.Stages = 4;
	Wheat.DaysPerStage = 2;

	int32 Stage = 0;
	int32 LastDay = 0;
	bool bWatered = true;

	TestFalse(TEXT("one day is before the first interval"),
		FarmLogic::AdvanceCrop(Wheat, 1, Stage, LastDay, bWatered, true));
	TestEqual(TEXT("stage stays 0 on day 1"), Stage, 0);
	TestTrue(TEXT("water is kept while waiting"), bWatered);

	bWatered = true;
	TestTrue(TEXT("day 2 completes the first interval"),
		FarmLogic::AdvanceCrop(Wheat, 2, Stage, LastDay, bWatered, true));
	TestEqual(TEXT("stage 1 after day 2"), Stage, 1);
	TestEqual(TEXT("growth day is 2"), LastDay, 2);
	TestFalse(TEXT("water is consumed when the stage moves"), bWatered);

	bWatered = true;
	TestTrue(TEXT("day 4 reaches stage 2"),
		FarmLogic::AdvanceCrop(Wheat, 4, Stage, LastDay, bWatered, true));
	TestEqual(TEXT("stage 2 after day 4"), Stage, 2);
	TestEqual(TEXT("growth day is 4"), LastDay, 4);

	bWatered = true;
	TestTrue(TEXT("day 6 reaches the last stage"),
		FarmLogic::AdvanceCrop(Wheat, 6, Stage, LastDay, bWatered, true));
	TestEqual(TEXT("stage 3 is the mature stage"), Stage, 3);
	TestTrue(TEXT("reports mature at stage 3"), FarmLogic::IsMature(Wheat, Stage));

	bWatered = true;
	TestFalse(TEXT("a mature crop does not advance further"),
		FarmLogic::AdvanceCrop(Wheat, 8, Stage, LastDay, bWatered, true));
	TestEqual(TEXT("stage stays 3"), Stage, 3);
	TestEqual(TEXT("growth day stays 6"), LastDay, 6);
	TestFalse(TEXT("water is cleared on the mature check"), bWatered);
	return true;
}

/** Two furnace steps in a row: two ore become two ingots and the timer carries over. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmExtraSmeltTwiceTest, "FarmGame.Extra.SmeltTwoStepsInARow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmExtraSmeltTwiceTest::RunTest(const FString& Parameters)
{
	int32 Ore = 3;
	int32 Ingots = 0;
	float Timer = 0.f;

	TestTrue(TEXT("first step smelts an ingot"),
		FarmLogic::SmeltStep(Ore, Ingots, Timer, 5.f, 4.f));
	TestEqual(TEXT("one ore left after the first step"), Ore, 2);
	TestEqual(TEXT("one ingot after the first step"), Ingots, 1);
	TestEqual(TEXT("timer keeps the first remainder"), Timer, 1.f, 0.001f);

	TestTrue(TEXT("second step smelts another ingot"),
		FarmLogic::SmeltStep(Ore, Ingots, Timer, 5.f, 4.f));
	TestEqual(TEXT("one ore left after the second step"), Ore, 1);
	TestEqual(TEXT("two ingots after the second step"), Ingots, 2);
	TestEqual(TEXT("timer keeps the second remainder"), Timer, 2.f, 0.001f);
	return true;
}

/** SellEverything sells the ingot but keeps seeds and raw resources. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmExtraSellIngotTest, "FarmGame.Extra.SellIncludesIngotKeepsSeedsAndResources",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmExtraSellIngotTest::RunTest(const FString& Parameters)
{
	UFarmInventoryComponent* Inventory = NewObject<UFarmInventoryComponent>();
	TestNotNull(TEXT("inventory created"), Inventory);
	if (!Inventory)
	{
		return false;
	}

	Inventory->ResetForNewGame();
	Inventory->Items.Reset();
	Inventory->AddItem(TEXT("carrot_seed"), 2);
	Inventory->AddItem(TEXT("wood"), 4);
	Inventory->AddItem(TEXT("ore"), 2);
	Inventory->AddItem(TEXT("hay"), 3);
	Inventory->AddItem(TEXT("ingot"), 2);
	Inventory->AddItem(TEXT("carrot"), 1);

	TestEqual(TEXT("only ingot and carrot are sellable"), Inventory->GetSellableCount(), 3);

	const int32 GoldBefore = Inventory->Gold;
	const int32 Earned = Inventory->SellEverything();
	TestEqual(TEXT("earned 2 ingots + 1 carrot"), Earned, 44);
	TestEqual(TEXT("gold increased by the sale"), Inventory->Gold, GoldBefore + 44);
	TestEqual(TEXT("ingots sold"), Inventory->GetCount(TEXT("ingot")), 0);
	TestEqual(TEXT("carrot sold"), Inventory->GetCount(TEXT("carrot")), 0);
	TestEqual(TEXT("seeds kept"), Inventory->GetCount(TEXT("carrot_seed")), 2);
	TestEqual(TEXT("wood kept"), Inventory->GetCount(TEXT("wood")), 4);
	TestEqual(TEXT("ore kept"), Inventory->GetCount(TEXT("ore")), 2);
	TestEqual(TEXT("hay kept"), Inventory->GetCount(TEXT("hay")), 3);
	return true;
}

/** A failed craft must not consume any ingredient, not even the ones already plentiful. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmExtraCraftFailureTest, "FarmGame.Extra.CraftFailureKeepsMaterials",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmExtraCraftFailureTest::RunTest(const FString& Parameters)
{
	UFarmInventoryComponent* Inventory = NewObject<UFarmInventoryComponent>();
	TestNotNull(TEXT("inventory created"), Inventory);
	if (!Inventory)
	{
		return false;
	}
	Inventory->ResetForNewGame();

	const FCraftRecipe* Furnace = FarmRecipes::Find(TEXT("furnace_kit"));
	TestNotNull(TEXT("furnace kit recipe exists"), Furnace);
	if (!Furnace)
	{
		return false;
	}

	TestEqual(TEXT("starting wood"), Inventory->GetCount(TEXT("wood")), 6);
	TestEqual(TEXT("no starting stone"), Inventory->GetCount(TEXT("stone")), 0);

	TestFalse(TEXT("cannot craft without stone"), Inventory->CanCraft(*Furnace));
	TestFalse(TEXT("craft fails without stone"), Inventory->Craft(*Furnace));
	TestEqual(TEXT("wood is not partially consumed"), Inventory->GetCount(TEXT("wood")), 6);
	TestEqual(TEXT("stone stays empty"), Inventory->GetCount(TEXT("stone")), 0);
	TestEqual(TEXT("no furnace kit produced"), Inventory->GetCount(TEXT("furnace_kit")), 0);

	Inventory->AddItem(TEXT("stone"), 5);
	TestFalse(TEXT("five stone is still not enough"), Inventory->Craft(*Furnace));
	TestEqual(TEXT("five stone kept"), Inventory->GetCount(TEXT("stone")), 5);
	TestEqual(TEXT("wood still untouched"), Inventory->GetCount(TEXT("wood")), 6);
	TestEqual(TEXT("still no furnace kit"), Inventory->GetCount(TEXT("furnace_kit")), 0);
	TestEqual(TEXT("gold unchanged"), Inventory->Gold, 20);
	return true;
}

/** SeedId maps all three crops to catalogued seed items. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmExtraSeedIdTest, "FarmGame.Extra.SeedIdsForThreeCrops",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmExtraSeedIdTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("carrot seed id"), FarmItems::SeedId(TEXT("carrot")), FName(TEXT("carrot_seed")));
	TestEqual(TEXT("potato seed id"), FarmItems::SeedId(TEXT("potato")), FName(TEXT("potato_seed")));
	TestEqual(TEXT("corn seed id"), FarmItems::SeedId(TEXT("corn")), FName(TEXT("corn_seed")));

	TArray<FName> SeedIds;
	SeedIds.Add(TEXT("carrot_seed"));
	SeedIds.Add(TEXT("potato_seed"));
	SeedIds.Add(TEXT("corn_seed"));

	for (const FName& SeedId : SeedIds)
	{
		TestNotNull(FString::Printf(TEXT("%s exists in the item catalog"), *SeedId.ToString()),
			FarmItems::Find(SeedId));
		TestTrue(FString::Printf(TEXT("%s is catalogued as a seed"), *SeedId.ToString()),
			FarmItems::IsSeed(SeedId));
	}
	return true;
}

/** Every recipe output must exist in the item catalog and produce at least one item. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmExtraRecipeOutputTest, "FarmGame.Extra.RecipeOutputsInCatalog",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmExtraRecipeOutputTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("recipe catalog is not empty"), FarmRecipes::All().Num() >= 4);

	for (const FCraftRecipe& Recipe : FarmRecipes::All())
	{
		TestNotNull(
			FString::Printf(TEXT("output %s exists in the item catalog"), *Recipe.OutputId.ToString()),
			FarmItems::Find(Recipe.OutputId));
		TestTrue(
			FString::Printf(TEXT("%s produces at least one item"), *Recipe.Id.ToString()),
			Recipe.OutputCount > 0);
	}

	TArray<FName> ExpectedOutputs;
	ExpectedOutputs.Add(TEXT("fence_kit"));
	ExpectedOutputs.Add(TEXT("furnace_kit"));
	ExpectedOutputs.Add(TEXT("ingot"));
	ExpectedOutputs.Add(TEXT("hay"));

	for (const FName& OutputId : ExpectedOutputs)
	{
		TestNotNull(FString::Printf(TEXT("expected output %s exists"), *OutputId.ToString()),
			FarmItems::Find(OutputId));
	}
	return true;
}

#endif
