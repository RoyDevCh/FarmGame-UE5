#include "FarmMenuWidget.h"
#include "FarmPlayerController.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "FarmMenu"

void SFarmMenuWidget::Construct(const FArguments& InArgs)
{
	PlayerController = InArgs._PlayerController;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.02f, 0.03f, 0.02f, 0.93f))
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 8)
				[
					SAssignNew(TitleText, STextBlock)
					.Text(LOCTEXT("Title", "FARM GAME"))
					.ColorAndOpacity(FLinearColor(1.f, 0.88f, 0.45f))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 42))
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 30)
				[
					SAssignNew(BodyText, STextBlock)
					.Text(LOCTEXT("Body", "WASD move  |  E interact  |  F attack  |  Q seed  |  B buy  |  C craft  |  G build  |  Tab bag  |  T sleep  |  Esc pause"))
					.ColorAndOpacity(FLinearColor(0.8f, 0.85f, 0.78f))
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 4)
				[
					SAssignNew(PrimaryButton, SButton)
					.OnClicked_Lambda([this]() { if (PlayerController.IsValid()) PlayerController->HandleMenuPrimary(); return FReply::Handled(); })
					[
						SNew(SBox).WidthOverride(260.f).HeightOverride(46.f)
						[
							SAssignNew(PrimaryLabel, STextBlock)
							.Text(LOCTEXT("Start", "Start game"))
							.Justification(ETextJustify::Center)
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 4)
				[
					SNew(SButton)
					.OnClicked_Lambda([this]() { if (PlayerController.IsValid()) PlayerController->HandleMenuQuit(); return FReply::Handled(); })
					[
						SNew(SBox).WidthOverride(260.f).HeightOverride(36.f)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Quit", "Quit"))
							.Justification(ETextJustify::Center)
						]
					]
				]
			]
		]
	];

	SetStartMode(true);
}

void SFarmMenuWidget::SetStartMode(bool bStart)
{
	if (TitleText.IsValid()) TitleText->SetText(bStart ? LOCTEXT("StartTitle", "FARM GAME") : LOCTEXT("PauseTitle", "PAUSED"));
	if (PrimaryLabel.IsValid()) PrimaryLabel->SetText(bStart ? LOCTEXT("StartGame", "Start game") : LOCTEXT("Resume", "Resume"));
}

FReply SFarmMenuWidget::OnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Escape && PlayerController.IsValid())
	{
		PlayerController->HandleMenuEscape();
		return FReply::Handled();
	}
	if ((Key == EKeys::Enter || Key == EKeys::SpaceBar) && PlayerController.IsValid())
	{
		PlayerController->HandleMenuPrimary();
		return FReply::Handled();
	}
	return SCompoundWidget::OnKeyDown(InGeometry, InKeyEvent);
}

#undef LOCTEXT_NAMESPACE
