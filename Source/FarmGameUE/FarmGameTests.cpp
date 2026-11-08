#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "FarmTypes.h"
#include "FarmInventoryComponent.h"
#include "FarmItems.h"
#include "FarmRecipes.h"
#include "FarmCropCatalog.h"
#include "FarmQuests.h"
#include "FarmPlayerController.h"
#include "FarmSaveGame.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmGrowthTest, "FarmGame.CropGrowth.AdvancesWithWater",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmGrowthTest::RunTest(const FString& Parameters)
{
	FCropDef Carrot;
	Carrot.Id = TEXT("carrot");
	Carrot.Stages = 3;
	Carrot.DaysPerStage = 1;

	int32 Stage = 0;
	int32 LastDay = 1;
	bool bWatered = true;

	const bool bAdvanced = FarmLogic::AdvanceCrop(Carrot, 2, Stage, LastDay, bWatered, true);
	TestTrue(TEXT("stage advances after one day when watered"), bAdvanced);
	TestEqual(TEXT("stage is 1"), Stage, 1);
	TestEqual(TEXT("growth day updated"), LastDay, 2);
	TestFalse(TEXT("water consumed"), bWatered);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmNoWaterTest, "FarmGame.CropGrowth.BlockedWithoutWater",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmNoWaterTest::RunTest(const FString& Parameters)
{
	FCropDef Potato;
	Potato.Id = TEXT("potato");
	Potato.Stages = 3;
	Potato.DaysPerStage = 2;

	int32 Stage = 0;
	int32 LastDay = 1;
	bool bWatered = false;

	TestFalse(TEXT("dry crop does not grow"), FarmLogic::AdvanceCrop(Potato, 2, Stage, LastDay, bWatered, false));
	TestEqual(TEXT("stage stays 0"), Stage, 0);

	int32 Stage2 = 0;
	int32 LastDay2 = 1;
	bool bWatered2 = true;
	TestFalse(TEXT("needs 2 days, not 1"), FarmLogic::AdvanceCrop(Potato, 2, Stage2, LastDay2, bWatered2, true));
	TestEqual(TEXT("stage still 0 after one day"), Stage2, 0);

	bWatered2 = true;
	TestTrue(TEXT("grows after two watered days"), FarmLogic::AdvanceCrop(Potato, 3, Stage2, LastDay2, bWatered2, true));
	TestEqual(TEXT("stage is 1"), Stage2, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmMatureTest, "FarmGame.CropGrowth.MaturesAndStops",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmMatureTest::RunTest(const FString& Parameters)
{
	FCropDef Corn;
	Corn.Id = TEXT("corn");
	Corn.Stages = 4;
	Corn.DaysPerStage = 1;

	int32 Stage = 0;
	int32 LastDay = 0;
	bool bWatered = true;
	for (int32 Day = 1; Day <= 10; ++Day)
	{
		bWatered = true;
		FarmLogic::AdvanceCrop(Corn, Day, Stage, LastDay, bWatered, true);
	}
	TestEqual(TEXT("stops at the last stage"), Stage, 3);
	TestTrue(TEXT("reports mature"), FarmLogic::IsMature(Corn, Stage));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmInventoryTest, "FarmGame.Inventory.BuyAndSell",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmInventoryTest::RunTest(const FString& Parameters)
{
	UFarmInventoryComponent* Inventory = NewObject<UFarmInventoryComponent>();
	TestNotNull(TEXT("inventory created"), Inventory);
	Inventory->ResetForNewGame();
	TestEqual(TEXT("starting gold"), Inventory->Gold, 20);
	TestEqual(TEXT("starting carrot seeds"), Inventory->GetCount(TEXT("carrot_seed")), 3);
	TestEqual(TEXT("starting wood"), Inventory->GetCount(TEXT("wood")), 6);

	TestTrue(TEXT("can buy carrot seed"), Inventory->BuyItem(TEXT("carrot_seed"), 1));
	TestEqual(TEXT("gold after buy"), Inventory->Gold, 17);
	TestEqual(TEXT("seed count after buy"), Inventory->GetCount(TEXT("carrot_seed")), 4);

	TestTrue(TEXT("remove one seed"), Inventory->RemoveItem(TEXT("carrot_seed"), 1));
	TestEqual(TEXT("seed count after remove"), Inventory->GetCount(TEXT("carrot_seed")), 3);

	Inventory->AddItem(TEXT("carrot"), 2);
	Inventory->AddItem(TEXT("ingot"), 1);
	const int32 Earned = Inventory->SellEverything();
	TestEqual(TEXT("earned gold (2 carrots + 1 ingot)"), Earned, 34);
	TestEqual(TEXT("gold after selling"), Inventory->Gold, 51);
	TestEqual(TEXT("produce cleared"), Inventory->GetCount(TEXT("carrot")), 0);
	TestEqual(TEXT("seeds kept"), Inventory->GetCount(TEXT("carrot_seed")), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmPoorTest, "FarmGame.Inventory.CannotBuyWithoutGold",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmPoorTest::RunTest(const FString& Parameters)
{
	UFarmInventoryComponent* Inventory = NewObject<UFarmInventoryComponent>();
	Inventory->ResetForNewGame();
	Inventory->Gold = 2;
	TestFalse(TEXT("cannot afford"), Inventory->BuyItem(TEXT("carrot_seed"), 1));
	TestEqual(TEXT("gold unchanged"), Inventory->Gold, 2);
	TestEqual(TEXT("no extra seeds"), Inventory->GetCount(TEXT("carrot_seed")), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmCraftTest, "FarmGame.Crafting.ConsumesIngredientsAndOutputs",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmCraftTest::RunTest(const FString& Parameters)
{
	UFarmInventoryComponent* Inventory = NewObject<UFarmInventoryComponent>();
	Inventory->ResetForNewGame();

	const FCraftRecipe* Fence = FarmRecipes::Find(TEXT("fence_kit"));
	TestNotNull(TEXT("fence recipe exists"), Fence);
	TestTrue(TEXT("can craft fence with 6 wood"), Inventory->CanCraft(*Fence));
	TestTrue(TEXT("craft fence"), Inventory->Craft(*Fence));
	TestEqual(TEXT("wood consumed"), Inventory->GetCount(TEXT("wood")), 1);
	TestEqual(TEXT("fence kit added"), Inventory->GetCount(TEXT("fence_kit")), 1);

	TestFalse(TEXT("cannot craft a second fence"), Inventory->CanCraft(*Fence));
	TestFalse(TEXT("craft fails without wood"), Inventory->Craft(*Fence));
	TestEqual(TEXT("still one fence kit"), Inventory->GetCount(TEXT("fence_kit")), 1);

	const FCraftRecipe* Ingot = FarmRecipes::Find(TEXT("ingot"));
	TestNotNull(TEXT("ingot recipe exists"), Ingot);
	TestTrue(TEXT("ingot recipe needs a furnace"), Ingot->bNeedsFurnace);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmItemsTest, "FarmGame.Items.CatalogIsConsistent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmItemsTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("catalog not empty"), FarmItems::All().Num() >= 10);
	TestNotNull(TEXT("carrot seed defined"), FarmItems::Find(TEXT("carrot_seed")));
	TestNotNull(TEXT("carrot defined"), FarmItems::Find(TEXT("carrot")));
	TestTrue(TEXT("carrot sells for 8"), FarmItems::SellPriceOf(TEXT("carrot")) == 8);
	TestTrue(TEXT("carrot seed is a seed"), FarmItems::IsSeed(TEXT("carrot_seed")));
	TestFalse(TEXT("carrot is not a seed"), FarmItems::IsSeed(TEXT("carrot")));
	TestEqual(TEXT("seed id derived"), FarmItems::SeedId(TEXT("corn")), FName(TEXT("corn_seed")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmHayTest, "FarmGame.Crafting.HayFromCorn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmHayTest::RunTest(const FString& Parameters)
{
	UFarmInventoryComponent* Inventory = NewObject<UFarmInventoryComponent>();
	Inventory->ResetForNewGame();
	Inventory->Items.Reset();
	Inventory->AddItem(TEXT("corn"), 2);

	const FCraftRecipe* Hay = FarmRecipes::Find(TEXT("hay_bundle"));
	TestNotNull(TEXT("hay recipe exists"), Hay);
	TestTrue(TEXT("can craft hay from 2 corn"), Inventory->Craft(*Hay));
	TestEqual(TEXT("corn consumed"), Inventory->GetCount(TEXT("corn")), 0);
	TestEqual(TEXT("hay produced"), Inventory->GetCount(TEXT("hay")), 2);
	TestFalse(TEXT("cannot craft again"), Inventory->Craft(*Hay));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmSellFilterTest, "FarmGame.Inventory.SellSkipsSeedsAndResources",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmSellFilterTest::RunTest(const FString& Parameters)
{
	UFarmInventoryComponent* Inventory = NewObject<UFarmInventoryComponent>();
	Inventory->ResetForNewGame();
	Inventory->Items.Reset();
	Inventory->AddItem(TEXT("wood"), 5);
	Inventory->AddItem(TEXT("ore"), 3);
	Inventory->AddItem(TEXT("egg"), 2);
	Inventory->AddItem(TEXT("milk"), 1);

	const int32 Before = Inventory->Gold;
	const int32 Earned = Inventory->SellEverything();
	TestEqual(TEXT("earned from egg + milk"), Earned, 36);
	TestEqual(TEXT("gold increased"), Inventory->Gold, Before + 36);
	TestEqual(TEXT("wood kept"), Inventory->GetCount(TEXT("wood")), 5);
	TestEqual(TEXT("ore kept"), Inventory->GetCount(TEXT("ore")), 3);
	TestEqual(TEXT("eggs sold"), Inventory->GetCount(TEXT("egg")), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmRecipeCatalogTest, "FarmGame.Recipes.CatalogReferencesKnownItems",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmRecipeCatalogTest::RunTest(const FString& Parameters)
{
	for (const FCraftRecipe& Recipe : FarmRecipes::All())
	{
		TestNotNull(FString::Printf(TEXT("output %s exists in item catalog"), *Recipe.OutputId.ToString()), FarmItems::Find(Recipe.OutputId));
		TestTrue(FString::Printf(TEXT("%s has inputs"), *Recipe.Id.ToString()), Recipe.Inputs.Num() > 0);
		for (const FCraftIngredient& Ingredient : Recipe.Inputs)
		{
			TestNotNull(FString::Printf(TEXT("ingredient %s exists"), *Ingredient.ItemId.ToString()), FarmItems::Find(Ingredient.ItemId));
			TestTrue(FString::Printf(TEXT("ingredient %s has positive count"), *Ingredient.ItemId.ToString()), Ingredient.Count > 0);
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmSmeltTest, "FarmGame.Crafting.FurnaceSmeltsOneOrePerInterval",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmSmeltTest::RunTest(const FString& Parameters)
{
	int32 Ore = 2;
	int32 Ingots = 0;
	float Timer = 0.f;

	TestFalse(TEXT("no ingot before the interval"), FarmLogic::SmeltStep(Ore, Ingots, Timer, 4.f, 8.f));
	TestEqual(TEXT("ore unchanged"), Ore, 2);
	TestTrue(TEXT("ingot after 8 seconds"), FarmLogic::SmeltStep(Ore, Ingots, Timer, 4.f, 8.f));
	TestEqual(TEXT("one ore consumed"), Ore, 1);
	TestEqual(TEXT("one ingot produced"), Ingots, 1);
	TestFalse(TEXT("no smelt with empty furnace"), FarmLogic::SmeltStep(Ore, Ingots, Timer, 100.f, 8.f) && Ore < 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmCropCatalogTest, "FarmGame.Crops.CatalogSanity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmCropCatalogTest::RunTest(const FString& Parameters)
{
	const TArray<FCropDef> Crops = FarmCrops::MakeDefaults();
	TestEqual(TEXT("five crops"), Crops.Num(), 5);

	TSet<FName> Ids;
	for (const FCropDef& Crop : Crops)
	{
		TestFalse(TEXT("crop id set"), Crop.Id.IsNone());
		TestFalse(TEXT("crop id unique"), Ids.Contains(Crop.Id));
		Ids.Add(Crop.Id);
		TestTrue(TEXT("at least two stages"), Crop.Stages >= 2);
		TestTrue(TEXT("at least one day per stage"), Crop.DaysPerStage >= 1);
		TestTrue(TEXT("harvest yields something"), Crop.YieldPerHarvest >= 1);
		TestTrue(FString::Printf(TEXT("%s is profitable"), *Crop.Id.ToString()), FarmCrops::ProfitPerHarvest(Crop) > 0);
		TestNotNull(FString::Printf(TEXT("%s seed in item catalog"), *Crop.Id.ToString()), FarmItems::Find(FarmItems::SeedId(Crop.Id)));
		TestNotNull(FString::Printf(TEXT("%s produce in item catalog"), *Crop.Id.ToString()), FarmItems::Find(Crop.Id));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmItemIdsTest, "FarmGame.Items.IdsAreUnique",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmItemIdsTest::RunTest(const FString& Parameters)
{
	TSet<FName> Ids;
	for (const FItemDef& Def : FarmItems::All())
	{
		TestFalse(TEXT("item id set"), Def.Id.IsNone());
		TestFalse(FString::Printf(TEXT("item id %s unique"), *Def.Id.ToString()), Ids.Contains(Def.Id));
		TestNotNull(TEXT("label set"), Def.Label);
		Ids.Add(Def.Id);
	}
	TestTrue(TEXT("at least 18 items"), FarmItems::All().Num() >= 18);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmRecipeIdsTest, "FarmGame.Recipes.IdsAreUnique",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmRecipeIdsTest::RunTest(const FString& Parameters)
{
	TSet<FName> Ids;
	for (const FCraftRecipe& Recipe : FarmRecipes::All())
	{
		TestFalse(TEXT("recipe id unique"), Ids.Contains(Recipe.Id));
		Ids.Add(Recipe.Id);
		TestTrue(TEXT("output count positive"), Recipe.OutputCount > 0);
	}
	TestTrue(TEXT("at least four recipes"), FarmRecipes::All().Num() >= 4);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmUnknownItemTest, "FarmGame.Inventory.UnknownItemRejected",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmUnknownItemTest::RunTest(const FString& Parameters)
{
	UFarmInventoryComponent* Inventory = NewObject<UFarmInventoryComponent>();
	Inventory->ResetForNewGame();
	const int32 GoldBefore = Inventory->Gold;
	TestFalse(TEXT("unknown item cannot be bought"), Inventory->BuyItem(TEXT("not_an_item"), 1));
	TestEqual(TEXT("gold unchanged"), Inventory->Gold, GoldBefore);
	TestEqual(TEXT("bag unchanged"), Inventory->GetCount(TEXT("not_an_item")), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmEmptySaleTest, "FarmGame.Inventory.EmptySaleIsNoop",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmEmptySaleTest::RunTest(const FString& Parameters)
{
	UFarmInventoryComponent* Inventory = NewObject<UFarmInventoryComponent>();
	Inventory->ResetForNewGame();
	Inventory->Items.Reset();
	const int32 GoldBefore = Inventory->Gold;
	TestEqual(TEXT("nothing earned"), Inventory->SellEverything(), 0);
	TestEqual(TEXT("gold unchanged"), Inventory->Gold, GoldBefore);
	TestEqual(TEXT("sellable count zero"), Inventory->GetSellableCount(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmExactCraftTest, "FarmGame.Crafting.ExactMaterialsConsumeAll",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmExactCraftTest::RunTest(const FString& Parameters)
{
	UFarmInventoryComponent* Inventory = NewObject<UFarmInventoryComponent>();
	Inventory->ResetForNewGame();
	Inventory->Items.Reset();
	Inventory->AddItem(TEXT("stone"), 10);
	Inventory->AddItem(TEXT("wood"), 5);

	const FCraftRecipe* Furnace = FarmRecipes::Find(TEXT("furnace_kit"));
	TestNotNull(TEXT("furnace recipe exists"), Furnace);
	TestTrue(TEXT("can craft with exact materials"), Inventory->Craft(*Furnace));
	TestEqual(TEXT("stone consumed exactly"), Inventory->GetCount(TEXT("stone")), 0);
	TestEqual(TEXT("wood consumed exactly"), Inventory->GetCount(TEXT("wood")), 0);
	TestEqual(TEXT("furnace kit produced"), Inventory->GetCount(TEXT("furnace_kit")), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmSaveDefaultsTest, "FarmGame.Save.DefaultsAreSane",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmSaveDefaultsTest::RunTest(const FString& Parameters)
{
	UFarmSaveGame* Save = NewObject<UFarmSaveGame>();
	TestNotNull(TEXT("save object created"), Save);
	TestEqual(TEXT("default day"), Save->Day, 1);
	TestEqual(TEXT("default gold"), Save->Gold, 20);
	TestEqual(TEXT("default health"), Save->PlayerHealth, 100);
	TestEqual(TEXT("no plots by default"), Save->Plots.Num(), 0);
	TestEqual(TEXT("no buildings by default"), Save->Buildings.Num(), 0);
	TestEqual(TEXT("no animals by default"), Save->Animals.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmRegrowTest, "FarmGame.Crops.RegrowAfterHarvest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmRegrowTest::RunTest(const FString& Parameters)
{
	const TArray<FCropDef> Crops = FarmCrops::MakeDefaults();
	for (const FCropDef& Crop : Crops)
	{
		const int32 Next = FarmLogic::StageAfterHarvest(Crop);
		if (Crop.Id == TEXT("tomato"))
		{
			TestEqual(TEXT("tomato regrows to stage 1"), Next, 1);
			TestTrue(TEXT("tomato flagged regrow"), Crop.bRegrowAfterHarvest);
		}
		else
		{
			TestEqual(FString::Printf(TEXT("%s is cleared after harvest"), *Crop.Id.ToString()), Next, -1);
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmStorageTest, "FarmGame.Inventory.ChestDepositWithdraw",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmStorageTest::RunTest(const FString& Parameters)
{
	UFarmInventoryComponent* Inventory = NewObject<UFarmInventoryComponent>();
	Inventory->ResetForNewGame();
	Inventory->Items.Reset();
	Inventory->AddItem(TEXT("wood"), 7);
	Inventory->AddItem(TEXT("stone"), 3);
	Inventory->AddItem(TEXT("carrot"), 2);
	Inventory->AddItem(TEXT("hay"), 1);

	TMap<FName, int32> Chest;
	const int32 Moved = Inventory->DepositResources(Chest);
	TestEqual(TEXT("deposited resources only"), Moved, 11);
	TestEqual(TEXT("chest wood"), Chest.FindRef(TEXT("wood")), 7);
	TestEqual(TEXT("chest stone"), Chest.FindRef(TEXT("stone")), 3);
	TestEqual(TEXT("chest hay"), Chest.FindRef(TEXT("hay")), 1);
	TestEqual(TEXT("carrots stay in bag"), Inventory->GetCount(TEXT("carrot")), 2);
	TestEqual(TEXT("wood left the bag"), Inventory->GetCount(TEXT("wood")), 0);

	const int32 Taken = Inventory->WithdrawResources(Chest);
	TestEqual(TEXT("withdrew everything"), Taken, 11);
	TestEqual(TEXT("bag wood restored"), Inventory->GetCount(TEXT("wood")), 7);
	TestEqual(TEXT("chest empty"), Chest.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmChestRecipeTest, "FarmGame.Crafting.ChestRecipeCostsWood",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmChestRecipeTest::RunTest(const FString& Parameters)
{
	const FCraftRecipe* Chest = FarmRecipes::Find(TEXT("chest_kit"));
	TestNotNull(TEXT("chest recipe exists"), Chest);
	TestEqual(TEXT("one input"), Chest->Inputs.Num(), 1);
	TestEqual(TEXT("input is wood"), Chest->Inputs[0].ItemId, FName(TEXT("wood")));
	TestEqual(TEXT("costs 12 wood"), Chest->Inputs[0].Count, 12);
	TestNotNull(TEXT("chest kit in catalog"), FarmItems::Find(TEXT("chest_kit")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmQuestAdvanceTest, "FarmGame.Quests.AdvanceToCompletion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmQuestAdvanceTest::RunTest(const FString& Parameters)
{
	TArray<FQuestState> Quests = FarmQuests::MakeDefault();
	TestEqual(TEXT("eight objectives"), Quests.Num(), 8);

	FQuestState& Quest = Quests[0];
	TestFalse(TEXT("no completion before target"), FarmQuests::Advance(Quest, 1));
	TestEqual(TEXT("progress tracked"), Quest.Progress, 1);
	TestFalse(TEXT("still short"), FarmQuests::Advance(Quest, 1));
	TestTrue(TEXT("completes on target"), FarmQuests::Advance(Quest, 1));
	TestEqual(TEXT("progress capped"), Quest.Progress, Quest.Target);
	TestFalse(TEXT("no double completion"), FarmQuests::Advance(Quest, 5));
	TestEqual(TEXT("progress stays capped"), Quest.Progress, Quest.Target);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmQuestDefaultsTest, "FarmGame.Quests.DefaultsReferenceItems",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmQuestDefaultsTest::RunTest(const FString& Parameters)
{
	for (const FQuestState& Quest : FarmQuests::MakeDefault())
	{
		TestTrue(TEXT("title set"), !Quest.Title.IsEmpty());
		TestTrue(TEXT("target positive"), Quest.Target > 0);
		TestNotNull(FString::Printf(TEXT("%s exists in item catalog"), *Quest.ItemId.ToString()), FarmItems::Find(Quest.ItemId));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmGatherYieldTest, "FarmGame.Tools.GatherYieldDoubles",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmGatherYieldTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("bare hands yield one"), FarmLogic::GatherYield(0), 1);
	TestEqual(TEXT("tool yields two"), FarmLogic::GatherYield(1), 2);
	TestEqual(TEXT("steel yields three"), FarmLogic::GatherYield(2), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmToolsCatalogTest, "FarmGame.Tools.CatalogHasAxeAndPickaxe",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmToolsCatalogTest::RunTest(const FString& Parameters)
{
	for (const TCHAR* Id : { TEXT("axe"), TEXT("pickaxe"), TEXT("steel_axe"), TEXT("steel_pickaxe") })
	{
		TestNotNull(FString::Printf(TEXT("%s in item catalog"), Id), FarmItems::Find(Id));
		const FCraftRecipe* Recipe = FarmRecipes::Find(Id);
		TestNotNull(FString::Printf(TEXT("%s recipe exists"), Id), Recipe);
		TestEqual(TEXT("recipe outputs the tool"), Recipe->OutputId, FName(Id));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmShopStockTest, "FarmGame.Shop.StockIsValid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmShopStockTest::RunTest(const FString& Parameters)
{
	const TArray<AFarmPlayerController::FShopEntry>& Stock = AFarmPlayerController::GetShopStock();
	TestTrue(TEXT("shop has goods"), Stock.Num() >= 8);
	for (const AFarmPlayerController::FShopEntry& Entry : Stock)
	{
		TestNotNull(FString::Printf(TEXT("%s exists"), *Entry.Id.ToString()), FarmItems::Find(Entry.Id));
		TestTrue(FString::Printf(TEXT("%s has a price"), *Entry.Id.ToString()), Entry.Price > 0);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmFoodCatalogTest, "FarmGame.Food.CatalogHasAppleAndBread",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmFoodCatalogTest::RunTest(const FString& Parameters)
{
	for (const TCHAR* Id : { TEXT("apple"), TEXT("bread") })
	{
		const FItemDef* Def = FarmItems::Find(Id);
		TestNotNull(FString::Printf(TEXT("%s in item catalog"), Id), Def);
		if (!Def)
		{
			continue;
		}
		TestTrue(FString::Printf(TEXT("%s is food"), Id), Def->Kind == EItemKind::Food);
		TestTrue(FString::Printf(TEXT("%s has a buy price"), Id), Def->BuyPrice > 0);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmShopFoodTest, "FarmGame.Food.ShopStockIncludesFood",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmShopFoodTest::RunTest(const FString& Parameters)
{
	const TArray<AFarmPlayerController::FShopEntry>& Stock = AFarmPlayerController::GetShopStock();
	for (const TCHAR* Id : { TEXT("apple"), TEXT("bread") })
	{
		const FName ItemId(Id);
		const AFarmPlayerController::FShopEntry* Entry = Stock.FindByPredicate(
			[ItemId](const AFarmPlayerController::FShopEntry& Candidate) { return Candidate.Id == ItemId; });
		TestNotNull(FString::Printf(TEXT("%s is stocked"), Id), Entry);
		if (!Entry)
		{
			continue;
		}
		TestTrue(FString::Printf(TEXT("%s has a positive price"), Id), Entry->Price > 0);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmSteelRecipeTest, "FarmGame.Tools.SteelRecipesConsumeBaseTool",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmSteelRecipeTest::RunTest(const FString& Parameters)
{
	struct FSteelCase
	{
		const TCHAR* RecipeId;
		const TCHAR* BaseToolId;
	};

	const FSteelCase Cases[] = {
		{ TEXT("steel_axe"), TEXT("axe") },
		{ TEXT("steel_pickaxe"), TEXT("pickaxe") }
	};

	for (const FSteelCase& Case : Cases)
	{
		const FCraftRecipe* Recipe = FarmRecipes::Find(Case.RecipeId);
		TestNotNull(FString::Printf(TEXT("%s recipe exists"), Case.RecipeId), Recipe);
		if (!Recipe)
		{
			continue;
		}
		TestEqual(FString::Printf(TEXT("%s has two inputs"), Case.RecipeId), Recipe->Inputs.Num(), 2);

		const FName BaseToolId(Case.BaseToolId);
		const FCraftIngredient* ToolIngredient = Recipe->Inputs.FindByPredicate(
			[BaseToolId](const FCraftIngredient& Ingredient) { return Ingredient.ItemId == BaseToolId; });
		TestNotNull(FString::Printf(TEXT("%s takes a %s"), Case.RecipeId, Case.BaseToolId), ToolIngredient);
		if (ToolIngredient)
		{
			TestEqual(FString::Printf(TEXT("%s consumes one %s"), Case.RecipeId, Case.BaseToolId), ToolIngredient->Count, 1);
		}

		const FCraftIngredient* OreIngredient = Recipe->Inputs.FindByPredicate(
			[](const FCraftIngredient& Ingredient) { return Ingredient.ItemId == FName(TEXT("ore")); });
		TestNotNull(FString::Printf(TEXT("%s takes ore"), Case.RecipeId), OreIngredient);
		if (OreIngredient)
		{
			TestEqual(FString::Printf(TEXT("%s consumes five ore"), Case.RecipeId), OreIngredient->Count, 5);
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmQuestLastObjectiveTest, "FarmGame.Quests.LastObjectiveIsSteelAxe",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmQuestLastObjectiveTest::RunTest(const FString& Parameters)
{
	const TArray<FQuestState> Quests = FarmQuests::MakeDefault();
	TestEqual(TEXT("eight objectives"), Quests.Num(), 8);
	if (Quests.Num() < 8)
	{
		return false;
	}
	TestEqual(TEXT("second to last objective is the apple"), Quests[6].ItemId, FName(TEXT("apple")));
	TestEqual(TEXT("last objective is the steel axe"), Quests[7].ItemId, FName(TEXT("steel_axe")));
	TestTrue(TEXT("steel axe quest needs one"), Quests[7].Target == 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmSpendGoldTest, "FarmGame.Inventory.SpendGoldIsAtomic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmSpendGoldTest::RunTest(const FString& Parameters)
{
	UFarmInventoryComponent* Inventory = NewObject<UFarmInventoryComponent>();
	TestNotNull(TEXT("inventory created"), Inventory);
	if (!Inventory)
	{
		return false;
	}
	Inventory->ResetForNewGame();
	TestEqual(TEXT("starting gold"), Inventory->Gold, 20);

	const int32 Before = Inventory->Gold;
	TestTrue(TEXT("can spend 5 gold"), Inventory->SpendGold(5));
	TestEqual(TEXT("gold reduced by five"), Inventory->Gold, Before - 5);

	const int32 AfterSmallSpend = Inventory->Gold;
	TestFalse(TEXT("cannot spend more gold than the wallet holds"), Inventory->SpendGold(999999));
	TestEqual(TEXT("gold unchanged after a failed spend"), Inventory->Gold, AfterSmallSpend);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmRecipeOutputsTest, "FarmGame.Recipes.AllOutputsExistInCatalog",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmRecipeOutputsTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("recipe catalog not empty"), FarmRecipes::All().Num() > 0);
	for (const FCraftRecipe& Recipe : FarmRecipes::All())
	{
		TestNotNull(FString::Printf(TEXT("output %s of %s exists in the item catalog"), *Recipe.OutputId.ToString(), *Recipe.Id.ToString()), FarmItems::Find(Recipe.OutputId));
		TestTrue(FString::Printf(TEXT("%s produces at least one item"), *Recipe.Id.ToString()), Recipe.OutputCount >= 1);
	}
	return true;
}

#endif
