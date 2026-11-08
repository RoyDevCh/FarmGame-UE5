#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "FarmStatsComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmStatsLevelCurveTest, "FarmGame.Stats.LevelCurve",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmStatsLevelCurveTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("0 XP is level 1"), FarmStatsLogic::LevelForXP(0), 1);
	TestEqual(TEXT("99 XP is still level 1"), FarmStatsLogic::LevelForXP(99), 1);
	TestEqual(TEXT("100 XP is level 2"), FarmStatsLogic::LevelForXP(100), 2);
	TestEqual(TEXT("250 XP is level 3"), FarmStatsLogic::LevelForXP(250), 3);

	TestEqual(TEXT("level 1 needs 0 XP"), FarmStatsLogic::XPForLevel(1), 0);
	TestEqual(TEXT("level 3 needs 200 XP"), FarmStatsLogic::XPForLevel(3), 200);
	TestEqual(TEXT("curve round-trips at 500 XP"), FarmStatsLogic::XPForLevel(FarmStatsLogic::LevelForXP(500)), 500);

	UFarmStatsComponent* Stats = NewObject<UFarmStatsComponent>();
	TestNotNull(TEXT("stats component created"), Stats);
	Stats->AddXP(250);
	TestEqual(TEXT("component XP after grant"), Stats->GetXP(), 250);
	TestEqual(TEXT("component level recomputed from total"), Stats->GetLevel(), 3);
	TestEqual(TEXT("XP missing for level 4"), Stats->GetXPToNextLevel(), 50);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmStatsClampTest, "FarmGame.Stats.Clamp",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmStatsClampTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("negative clamps to 0"), FarmStatsLogic::ClampStat(-5.0f), 0.0f);
	TestEqual(TEXT("over max clamps to 100"), FarmStatsLogic::ClampStat(150.0f), 100.0f);
	TestEqual(TEXT("in range is unchanged"), FarmStatsLogic::ClampStat(42.0f), 42.0f);

	TestEqual(TEXT("custom max clamps down"), FarmStatsLogic::ClampToMax(150.0f, 60.0f), 60.0f);
	TestEqual(TEXT("custom max clamps up"), FarmStatsLogic::ClampToMax(-5.0f, 60.0f), 0.0f);

	UFarmStatsComponent* Stats = NewObject<UFarmStatsComponent>();
	TestNotNull(TEXT("stats component created"), Stats);
	Stats->MaxHunger = 60.0f;
	Stats->Hunger = 55.0f;
	Stats->Feed(30.0f);
	TestEqual(TEXT("feeding respects MaxHunger"), Stats->GetHunger(), 60.0f);

	Stats->Stamina = 10.0f;
	TestFalse(TEXT("cannot spend more stamina than available"), Stats->ConsumeStamina(25.0f));
	TestEqual(TEXT("failed spend leaves stamina alone"), Stats->GetStamina(), 10.0f);

	Stats->Stamina = 5.0f;
	TestTrue(TEXT("can spend available stamina"), Stats->ConsumeStamina(5.0f));
	TestEqual(TEXT("stamina floors at 0"), Stats->GetStamina(), 0.0f);
	return true;
}

#endif
