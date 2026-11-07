#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "FarmTypes.h"
#include "FarmInventoryComponent.h"

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

	const bool bAdvanced = FarmLogic::AdvanceCrop(Potato, 2, Stage, LastDay, bWatered, false);
	TestFalse(TEXT("dry crop does not grow"), bAdvanced);
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
	int32 Today = 0;
	for (int32 Day = 1; Day <= 10; ++Day)
	{
		Today = Day;
		bWatered = true;
		FarmLogic::AdvanceCrop(Corn, Today, Stage, LastDay, bWatered, true);
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
	TestEqual(TEXT("starting carrot seeds"), Inventory->GetSeedCount(TEXT("carrot")), 3);

	TestTrue(TEXT("can buy carrot seed"), Inventory->BuySeed(TEXT("carrot"), 3));
	TestEqual(TEXT("gold after buy"), Inventory->Gold, 17);
	TestEqual(TEXT("seed count after buy"), Inventory->GetSeedCount(TEXT("carrot")), 4);

	TestTrue(TEXT("consume one seed"), Inventory->ConsumeSeed(TEXT("carrot")));
	TestEqual(TEXT("seed count after consume"), Inventory->GetSeedCount(TEXT("carrot")), 3);

	FCropDef Carrot;
	Carrot.Id = TEXT("carrot");
	Carrot.SellPrice = 8;
	TArray<FCropDef> Crops;
	Crops.Add(Carrot);

	Inventory->AddProduce(TEXT("carrot"), 2);
	const int32 Earned = Inventory->SellAllProduce(Crops);
	TestEqual(TEXT("earned gold"), Earned, 16);
	TestEqual(TEXT("gold after selling"), Inventory->Gold, 33);
	TestEqual(TEXT("produce cleared"), Inventory->GetTotalProduce(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmPoorTest, "FarmGame.Inventory.CannotBuyWithoutGold",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmPoorTest::RunTest(const FString& Parameters)
{
	UFarmInventoryComponent* Inventory = NewObject<UFarmInventoryComponent>();
	Inventory->ResetForNewGame();
	Inventory->Gold = 2;
	TestFalse(TEXT("cannot afford"), Inventory->BuySeed(TEXT("carrot"), 3));
	TestEqual(TEXT("gold unchanged"), Inventory->Gold, 2);
	TestEqual(TEXT("no extra seeds"), Inventory->GetSeedCount(TEXT("carrot")), 3);
	return true;
}

#endif
