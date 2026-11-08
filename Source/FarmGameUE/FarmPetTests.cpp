#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "FarmPet.h"

#if WITH_DEV_AUTOMATION_TESTS

/** Follow -> Stay -> Follow, and a staying pet is stable under two toggles. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmPetModeTogglesTest, "FarmGame.Pet.ModeToggles",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmPetModeTogglesTest::RunTest(const FString& Parameters)
{
	FarmPetLogic::EPetMode Mode = FarmPetLogic::EPetMode::Follow;
	Mode = FarmPetLogic::ToggleMode(Mode);
	TestTrue(TEXT("Follow toggles to Stay"), Mode == FarmPetLogic::EPetMode::Stay);
	Mode = FarmPetLogic::ToggleMode(Mode);
	TestTrue(TEXT("Stay toggles back to Follow"), Mode == FarmPetLogic::EPetMode::Follow);

	FarmPetLogic::EPetMode StayMode = FarmPetLogic::EPetMode::Stay;
	StayMode = FarmPetLogic::ToggleMode(StayMode);
	TestTrue(TEXT("Stay toggles to Follow"), StayMode == FarmPetLogic::EPetMode::Follow);
	StayMode = FarmPetLogic::ToggleMode(StayMode);
	TestTrue(TEXT("Stay is Stay again after two toggles"), StayMode == FarmPetLogic::EPetMode::Stay);
	return true;
}

/** Taming needs Roll < Threshold on the 0..99 roll. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmPetTameThresholdTest, "FarmGame.Pet.TameThreshold",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmPetTameThresholdTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("roll 0 is below threshold 50"), FarmPetLogic::TryTame(0, 50));
	TestTrue(TEXT("roll 49 is below threshold 50"), FarmPetLogic::TryTame(49, 50));
	TestFalse(TEXT("roll 50 is not below threshold 50"), FarmPetLogic::TryTame(50, 50));
	TestFalse(TEXT("roll 99 is not below threshold 50"), FarmPetLogic::TryTame(99, 50));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
