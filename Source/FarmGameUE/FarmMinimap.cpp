#include "FarmMinimap.h"
#include "GameFramework/HUD.h"

namespace FarmMinimap
{
	namespace
	{
		/** A marker square centered on a mapped point, nudged so it never leaves the panel. */
		void DrawMarker(AHUD* HUD, const FVector2D& TopLeft, const FVector2D& Size, const FVector2D& Mapped,
			float MarkerSize, const FLinearColor& Color)
		{
			const FVector2D Origin = MarkerOrigin(Mapped, MarkerSize);
			const double MaxX = FMath::Max(0.0, static_cast<double>(Size.X) - MarkerSize);
			const double MaxY = FMath::Max(0.0, static_cast<double>(Size.Y) - MarkerSize);
			const float X = static_cast<float>(FMath::Clamp(static_cast<double>(Origin.X), 0.0, MaxX));
			const float Y = static_cast<float>(FMath::Clamp(static_cast<double>(Origin.Y), 0.0, MaxY));

			HUD->DrawRect(Color,
				static_cast<float>(TopLeft.X) + X,
				static_cast<float>(TopLeft.Y) + Y,
				MarkerSize,
				MarkerSize);
		}
	}

	void Draw(AHUD* HUD, const FVector& PlayerLocation, const TArray<FVector>& Plots, const TArray<FVector>& Buildings,
		const FVector& Center, float Extent, const FVector2D& TopLeft, const FVector2D& Size)
	{
		if (!HUD || Size.X <= 0.0 || Size.Y <= 0.0)
		{
			return;
		}

		const float X = static_cast<float>(TopLeft.X);
		const float Y = static_cast<float>(TopLeft.Y);
		const float W = static_cast<float>(Size.X);
		const float H = static_cast<float>(Size.Y);

		// Filled frame first, then the panel inset by the border width: that keeps the
		// border exactly BorderThickness pixels wide without drawing four separate edges.
		HUD->DrawRect(BorderColor(), X, Y, W, H);
		HUD->DrawRect(BackgroundColor(),
			X + BorderThickness,
			Y + BorderThickness,
			FMath::Max(0.f, W - BorderThickness * 2.f),
			FMath::Max(0.f, H - BorderThickness * 2.f));

		for (const FVector& Plot : Plots)
		{
			DrawMarker(HUD, TopLeft, Size, WorldToMap(Plot, Center, Extent, Size), PlotMarkerSize, PlotColor());
		}

		for (const FVector& Building : Buildings)
		{
			DrawMarker(HUD, TopLeft, Size, WorldToMap(Building, Center, Extent, Size), BuildingMarkerSize, BuildingColor());
		}

		// Player last so it is never hidden by a plot or building marker.
		DrawMarker(HUD, TopLeft, Size, WorldToMap(PlayerLocation, Center, Extent, Size), PlayerMarkerSize, PlayerColor());
	}
}
