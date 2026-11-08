#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "FarmWater.h"
#include "FarmLadder.h"

#if WITH_DEV_AUTOMATION_TESTS

/** Water slows the farmer to 45% of the walk speed; dry land keeps full speed. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmTraversalSwimSpeedTest, "FarmGame.Traversal.SwimSpeed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmTraversalSwimSpeedTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("swimming runs at 0.45x"),
		FarmWaterLogic::SpeedMultiplier(true), 0.45f);
	TestEqual(TEXT("dry land runs at 1.0x"),
		FarmWaterLogic::SpeedMultiplier(false), 1.0f);
	return true;
}

/** Ladder beats water, water beats plain ground, and neither maps to None. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmTraversalModeSelectionTest, "FarmGame.Traversal.ModeSelection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmTraversalModeSelectionTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("water only -> swim"),
		static_cast<int32>(FarmWaterLogic::TraversalFor(true, false)),
		static_cast<int32>(FarmWaterLogic::ETraversal::Swim));
	TestEqual(TEXT("ladder only -> climb"),
		static_cast<int32>(FarmWaterLogic::TraversalFor(false, true)),
		static_cast<int32>(FarmWaterLogic::ETraversal::Climb));
	TestEqual(TEXT("neither -> none"),
		static_cast<int32>(FarmWaterLogic::TraversalFor(false, false)),
		static_cast<int32>(FarmWaterLogic::ETraversal::None));
	TestEqual(TEXT("ladder wins inside water"),
		static_cast<int32>(FarmWaterLogic::TraversalFor(true, true)),
		static_cast<int32>(FarmWaterLogic::ETraversal::Climb));
	return true;
}

#endif
