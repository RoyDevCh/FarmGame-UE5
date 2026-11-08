#include "FarmSettingsWidget.h"
#include "FarmPlayerController.h"

#include "GameFramework/GameUserSettings.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "FarmSettings"

void SFarmSettingsWidget::Construct(const FArguments& InArgs)
{
	PlayerController = InArgs._PlayerController;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.03f, 0.04f, 0.03f, 0.94f))
		[
			SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 14)
				[
					SNew(STextBlock).Text(LOCTEXT("Title", "SETTINGS"))
					.ColorAndOpacity(FLinearColor(1.f, 0.88f, 0.45f))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 30))
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 4)
				[
					SNew(SButton)
					.OnClicked_Lambda([this]() { if (PlayerController.IsValid()) PlayerController->ToggleFullscreen(); return FReply::Handled(); })
					[
						SNew(SBox).WidthOverride(280.f).HeightOverride(38.f)
						[
							SAssignNew(FullscreenLabel, STextBlock).Text(LOCTEXT("Fullscreen", "Fullscreen")).Justification(ETextJustify::Center)
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 4)
				[
					SNew(SButton)
					.OnClicked_Lambda([this]() { if (PlayerController.IsValid()) PlayerController->CycleQuality(); return FReply::Handled(); })
					[
						SNew(SBox).WidthOverride(280.f).HeightOverride(38.f)
						[
							SAssignNew(QualityLabel, STextBlock).Text(LOCTEXT("Quality", "Quality")).Justification(ETextJustify::Center)
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 14, 0, 0)
				[
					SNew(SButton)
					.OnClicked_Lambda([this]() { if (PlayerController.IsValid()) PlayerController->CloseSettings(); return FReply::Handled(); })
					[
						SNew(SBox).WidthOverride(200.f).HeightOverride(34.f)
						[
							SNew(STextBlock).Text(LOCTEXT("Back", "Back (Esc)")).Justification(ETextJustify::Center)
						]
					]
				]
			]
		]
	];

	Refresh();
}

void SFarmSettingsWidget::Refresh()
{
	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!Settings) return;
	if (FullscreenLabel.IsValid())
	{
		const bool bFullscreen = Settings->GetFullscreenMode() != EWindowMode::Windowed;
		FullscreenLabel->SetText(FText::FromString(FString::Printf(TEXT("Fullscreen: %s"), bFullscreen ? TEXT("on") : TEXT("off"))));
	}
	if (QualityLabel.IsValid())
	{
		int32 Level = Settings->GetOverallScalabilityLevel();
		if (Level < 0) Level = 2;
		QualityLabel->SetText(FText::FromString(FString::Printf(TEXT("Quality: %d / 3"), Level)));
	}
}

FReply SFarmSettingsWidget::OnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape && PlayerController.IsValid())
	{
		PlayerController->CloseSettings();
		return FReply::Handled();
	}
	return SCompoundWidget::OnKeyDown(InGeometry, InKeyEvent);
}

#undef LOCTEXT_NAMESPACE
