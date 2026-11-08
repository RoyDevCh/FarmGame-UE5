#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "FarmRideable.h"

#if WITH_DEV_AUTOMATION_TESTS

/** Mounting needs a free saddle and a farmer strictly inside the radius. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmRideCanMountRulesTest, "FarmGame.Ride.CanMountRules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmRideCanMountRulesTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("a free mount accepts a rider inside the radius"),
		FarmRideLogic::CanMount(false, 100.f, 260.f));
	TestFalse(TEXT("an already ridden mount refuses a second rider"),
		FarmRideLogic::CanMount(true, 100.f, 260.f));
	TestFalse(TEXT("the exact radius is already out of reach"),
		FarmRideLogic::CanMount(false, 260.f, 260.f));
	return true;
}

/** Inside/outside tolerance at the radius rim, including degenerate radii. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmRideRadiusEdgesTest, "FarmGame.Ride.RadiusEdges",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmRideRadiusEdgesTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("just inside the radius mounts"),
		FarmRideLogic::CanMount(false, 259.9f, 260.f));
	TestFalse(TEXT("just outside the radius refuses"),
		FarmRideLogic::CanMount(false, 260.1f, 260.f));
	TestTrue(TEXT("standing on the mount mounts"),
		FarmRideLogic::CanMount(false, 0.f, 260.f));
	TestTrue(TEXT("a wider radius reaches farther"),
		FarmRideLogic::CanMount(false, 500.f, 1000.f));
	TestFalse(TEXT("a zero radius never mounts"),
		FarmRideLogic::CanMount(false, 0.f, 0.f));
	TestFalse(TEXT("a negative radius never mounts"),
		FarmRideLogic::CanMount(false, 100.f, -50.f));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
