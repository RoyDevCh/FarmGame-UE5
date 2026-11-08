#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "FarmAmbience.h"

#if WITH_DEV_AUTOMATION_TESTS

/** Clock picks daytime in the morning and night late in the day; rain picks the rain bed. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmAmbienceTrackSelectionTest, "FarmGame.Ambience.TrackSelection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmAmbienceTrackSelectionTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("0.1 is still the day bed"),
		static_cast<int32>(FarmAmbienceLogic::TrackFor(0.1f, false)),
		static_cast<int32>(FarmAmbienceLogic::ETrack::Day));
	TestEqual(TEXT("0.95 is the night bed"),
		static_cast<int32>(FarmAmbienceLogic::TrackFor(0.95f, false)),
		static_cast<int32>(FarmAmbienceLogic::ETrack::Night));
	TestEqual(TEXT("rain at 0.5 picks the rain bed"),
		static_cast<int32>(FarmAmbienceLogic::TrackFor(0.5f, true)),
		static_cast<int32>(FarmAmbienceLogic::ETrack::Rain));
	return true;
}

/** Rain overrides the clock in the morning and at night. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmAmbienceRainOverridesTest, "FarmGame.Ambience.RainOverrides",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmAmbienceRainOverridesTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("rain in the morning beats the day bed"),
		static_cast<int32>(FarmAmbienceLogic::TrackFor(0.1f, true)),
		static_cast<int32>(FarmAmbienceLogic::ETrack::Rain));
	TestEqual(TEXT("rain at night beats the night bed"),
		static_cast<int32>(FarmAmbienceLogic::TrackFor(0.95f, true)),
		static_cast<int32>(FarmAmbienceLogic::ETrack::Rain));
	return true;
}

#endif
