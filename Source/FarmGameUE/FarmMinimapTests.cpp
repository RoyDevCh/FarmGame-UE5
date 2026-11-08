#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "FarmMinimap.h"

#if WITH_DEV_AUTOMATION_TESTS

/** A world point sitting on the map center lands in the middle of the panel. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmMinimapCenterTest, "FarmGame.Minimap.CenterMapsToMiddle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmMinimapCenterTest::RunTest(const FString& Parameters)
{
	const FVector Center(1000.f, -500.f, 0.f);
	const FVector2D Size(200.f, 200.f);
	const FVector2D Mapped = FarmMinimap::WorldToMap(Center, Center, 1000.f, Size);

	TestEqual(TEXT("center X maps to half the width"), static_cast<float>(Mapped.X), 100.f);
	TestEqual(TEXT("center Y maps to half the height"), static_cast<float>(Mapped.Y), 100.f);
	return true;
}

/** Anything outside the covered world area is clamped to the panel edges. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmMinimapClampTest, "FarmGame.Minimap.ClampsOutside",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmMinimapClampTest::RunTest(const FString& Parameters)
{
	const FVector Center = FVector::ZeroVector;
	const FVector2D Size(200.f, 200.f);

	const FVector2D FarPositive = FarmMinimap::WorldToMap(Center + FVector(5000.f, 5000.f, 0.f), Center, 1000.f, Size);
	TestEqual(TEXT("far positive X clamps to the right edge"), static_cast<float>(FarPositive.X), 200.f);
	TestEqual(TEXT("far positive Y clamps to the bottom edge"), static_cast<float>(FarPositive.Y), 200.f);

	const FVector2D FarNegative = FarmMinimap::WorldToMap(Center - FVector(5000.f, 5000.f, 0.f), Center, 1000.f, Size);
	TestEqual(TEXT("far negative X clamps to the left edge"), static_cast<float>(FarNegative.X), 0.f);
	TestEqual(TEXT("far negative Y clamps to the top edge"), static_cast<float>(FarNegative.Y), 0.f);
	return true;
}

#endif
