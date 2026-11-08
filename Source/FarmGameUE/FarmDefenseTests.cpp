#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "FarmDefense.h"

#if WITH_DEV_AUTOMATION_TESTS

/** A target on the radius edge is outside; anything closer is inside. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmDefenseRangeChecksTest, "FarmGame.Defense.RangeChecks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmDefenseRangeChecksTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("100 is inside 200"), FarmDefenseLogic::InRange(100.f, 200.f));
	TestTrue(TEXT("199 is inside 200"), FarmDefenseLogic::InRange(199.f, 200.f));
	TestFalse(TEXT("200 is not inside 200"), FarmDefenseLogic::InRange(200.f, 200.f));
	TestFalse(TEXT("a zero radius has no inside"), FarmDefenseLogic::InRange(0.f, 0.f));
	return true;
}

/** The tower waits for the full cooldown; exact elapsed time counts as ready. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmDefenseCooldownChecksTest, "FarmGame.Defense.CooldownChecks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmDefenseCooldownChecksTest::RunTest(const FString& Parameters)
{
	TestFalse(TEXT("1s of a 1.5s cooldown is not ready"), FarmDefenseLogic::ReadyToFire(10.f, 9.f, 1.5f));
	TestTrue(TEXT("2s of a 1.5s cooldown is ready"), FarmDefenseLogic::ReadyToFire(11.f, 9.f, 1.5f));
	TestTrue(TEXT("exactly 1.5s elapsed is ready"), FarmDefenseLogic::ReadyToFire(10.5f, 9.f, 1.5f));
	return true;
}

#endif
