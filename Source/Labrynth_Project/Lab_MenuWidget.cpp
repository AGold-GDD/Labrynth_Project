#include "Lab_MenuWidget.h"
#include "Lab_GameInstance.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"

TSharedRef<SWidget> ULab_MenuWidget::RebuildWidget()
{
	return SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(340.f)
			[
				SNew(SVerticalBox)

				// Title
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.f, 0.f, 0.f, 32.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("LABRYNTH")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 28))
				]

				// HOST button
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 12.f)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.OnClicked(FOnClicked::CreateUObject(this, &ULab_MenuWidget::OnHostClicked))
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("HOST GAME")))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
					]
				]

				// IP input
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 12.f)
				[
					SAssignNew(IPInputBox, SEditableTextBox)
					.HintText(FText::FromString(TEXT("Enter Host IP...")))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
				]

				// JOIN button
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.OnClicked(FOnClicked::CreateUObject(this, &ULab_MenuWidget::OnJoinClicked))
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("JOIN GAME")))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
					]
				]
			]
		];
}

FReply ULab_MenuWidget::OnHostClicked()
{
	if (ULab_GameInstance* GI = GetGameInstance<ULab_GameInstance>())
	{
		RemoveFromParent();
		GI->HostGame(TEXT("/Game/MultiplayerStuff/lvl1"), 3);
	}
	return FReply::Handled();
}

FReply ULab_MenuWidget::OnJoinClicked()
{
	if (!IPInputBox.IsValid()) return FReply::Handled();

	const FString IP = IPInputBox->GetText().ToString().TrimStartAndEnd();
	if (IP.IsEmpty()) return FReply::Handled();

	if (ULab_GameInstance* GI = GetGameInstance<ULab_GameInstance>())
	{
		RemoveFromParent();
		GI->JoinGameByIP(IP);
	}
	return FReply::Handled();
}
