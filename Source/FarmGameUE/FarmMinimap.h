#pragma once

#include "CoreMinimal.h"

class AHUD;

/**
 * Minimap drawing for the Canvas HUD (FarmHUD::DrawHUD).
 * Every decision that needs no world lives here as inline code so the automation
 * tests can exercise it without spawning a HUD; Draw() is the only canvas access.
 */
namespace FarmMinimap
{
	/** Marker edge lengths in pixels: plots are small, buildings slightly larger, the player biggest. */
	inline constexpr float PlotMarkerSize = 3.f;
	inline constexpr float BuildingMarkerSize = 4.f;
	inline constexpr float PlayerMarkerSize = 5.f;

	/** Width of the frame around the map, in pixels. */
	inline constexpr float BorderThickness = 1.f;

	/** Canvas colors. Functions rather than globals so the header carries no static state. */
	inline FLinearColor BackgroundColor() { return FLinearColor(0.04f, 0.06f, 0.04f, 0.55f); }
	inline FLinearColor BorderColor() { return FLinearColor(0.62f, 0.66f, 0.6f, 0.85f); }
	inline FLinearColor PlotColor() { return FLinearColor(0.45f, 0.3f, 0.15f, 1.f); }
	inline FLinearColor BuildingColor() { return FLinearColor(0.8f, 0.8f, 0.82f, 1.f); }
	inline FLinearColor PlayerColor() { return FLinearColor(1.f, 0.92f, 0.25f, 1.f); }

	/**
	 * One axis of WorldToMap. Normalizes the offset from the center into -1..1,
	 * places it on the panel, then clamps to the panel edges. A non-positive Extent
	 * collapses everything onto the middle instead of dividing by zero.
	 */
	inline float MapAxis(double WorldCoord, double CenterCoord, double Extent, double SizeAxis)
	{
		const double Middle = SizeAxis * 0.5;
		if (Extent <= 0.0)
		{
			return static_cast<float>(Middle);
		}
		const double Normalized = (WorldCoord - CenterCoord) / Extent;
		return static_cast<float>(FMath::Clamp(Middle + Normalized * Middle, 0.0, SizeAxis));
	}

	/**
	 * Maps a world position (X/Y used, Z ignored) onto the map rectangle.
	 * Extent is the half-size of the covered world area in units, so the map shows
	 * Center +/- Extent on both axes; Size is the rectangle size in pixels. The result
	 * is always inside [0, Size] - positions outside the covered area stick to the edge.
	 */
	inline FVector2D WorldToMap(const FVector& World, const FVector& Center, float Extent, const FVector2D& Size)
	{
		return FVector2D(
			MapAxis(World.X, Center.X, Extent, Size.X),
			MapAxis(World.Y, Center.Y, Extent, Size.Y));
	}

	/** Top-left corner that centers a MarkerSize square on an already mapped point. */
	inline FVector2D MarkerOrigin(const FVector2D& Mapped, float MarkerSize)
	{
		const float Half = MarkerSize * 0.5f;
		return Mapped - FVector2D(Half, Half);
	}

	/**
	 * Draws the minimap panel at TopLeft (screen pixels): dark translucent square,
	 * 1px border, plots as brown 3x3 squares, buildings as light gray 4x4 squares and
	 * the player as a yellow 5x5 dot on top. Does nothing when HUD is null or Size is
	 * degenerate. Safe to call every frame; allocates nothing.
	 */
	void Draw(AHUD* HUD, const FVector& PlayerLocation, const TArray<FVector>& Plots, const TArray<FVector>& Buildings,
		const FVector& Center, float Extent, const FVector2D& TopLeft, const FVector2D& Size);
}
