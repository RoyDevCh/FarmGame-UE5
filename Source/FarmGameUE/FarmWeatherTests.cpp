#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "FarmWeather.h"

#if WITH_DEV_AUTOMATION_TESTS

/** Seasons last 28 days and repeat every 112 days. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmWeatherSeasonRollsTest, "FarmGame.Weather.SeasonRolls",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmWeatherSeasonRollsTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("day 1 is Spring"), FarmWeather::SeasonForDay(1) == FarmWeather::ESeason::Spring);
	TestTrue(TEXT("day 28 is still Spring"), FarmWeather::SeasonForDay(28) == FarmWeather::ESeason::Spring);
	TestTrue(TEXT("day 29 is Summer"), FarmWeather::SeasonForDay(29) == FarmWeather::ESeason::Summer);
	TestTrue(TEXT("day 57 is Autumn"), FarmWeather::SeasonForDay(57) == FarmWeather::ESeason::Autumn);
	TestTrue(TEXT("day 85 is Winter"), FarmWeather::SeasonForDay(85) == FarmWeather::ESeason::Winter);
	TestTrue(TEXT("day 113 wraps back to Spring"), FarmWeather::SeasonForDay(113) == FarmWeather::ESeason::Spring);
	return true;
}

/** Rain uses a fixed formula, so the same inputs always produce the same answer. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmWeatherRainDeterministicTest, "FarmGame.Weather.RainDeterministic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmWeatherRainDeterministicTest::RunTest(const FString& Parameters)
{
	const bool bExpected = ((3 * 37 + 10) % 100) < 30;

	TestTrue(TEXT("RollRain(3, 10) matches ((3 * 37 + 10) % 100) < 30"), FarmWeather::RollRain(3, 10) == bExpected);
	TestTrue(TEXT("repeating the same roll gives the same answer"), FarmWeather::RollRain(3, 10) == FarmWeather::RollRain(3, 10));
	TestTrue(TEXT("Winter grows crops slower than Spring"),
		FarmWeather::GrowthSpeed(FarmWeather::ESeason::Winter) < FarmWeather::GrowthSpeed(FarmWeather::ESeason::Spring));
	return true;
}

#endif
