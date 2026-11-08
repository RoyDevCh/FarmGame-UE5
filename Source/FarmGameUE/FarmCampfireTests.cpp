#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "FarmCampfire.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * A campfire only warms the farmer while it is lit and the farmer stands
 * strictly inside the radius; the rim itself is already too far.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmCampfireShouldHealTest, "FarmGame.Campfire.HealsOnlyWhenLit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmCampfireShouldHealTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("a lit fire heals a farmer inside the radius"),
		FarmCampfireLogic::ShouldHeal(true, 100.f, 320.f));
	TestFalse(TEXT("an unlit fire never heals"),
		FarmCampfireLogic::ShouldHeal(false, 100.f, 320.f));
	TestFalse(TEXT("the exact rim is outside the radius"),
		FarmCampfireLogic::ShouldHeal(true, 320.f, 320.f));
	return true;
}

/** Lit fires heal over time; put out they restore nothing. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmCampfireRatePositiveTest, "FarmGame.Campfire.RatePositive",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmCampfireRatePositiveTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("a lit campfire has a positive heal rate"),
		FarmCampfireLogic::HealPerSecond(true) > 0.f);
	TestEqual(TEXT("an unlit campfire heals nothing"),
		FarmCampfireLogic::HealPerSecond(false), 0.f);
	return true;
}

#endif
