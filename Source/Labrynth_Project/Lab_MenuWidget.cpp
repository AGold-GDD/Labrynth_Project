#include "Lab_MenuWidget.h"
#include "Lab_GameInstance.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"

TSharedRef<SWidget> ULab_MenuWidget::RebuildWidget()
{
	// Populate map options from the GameInstance's MapPool
	MapOptions.Empty();
	if (ULab_GameInstance* GI = GetGameInstance<ULab_GameInstance>())
	{
		for (const FString& Path : GI->MapPool)
		{
			// Display just the last segment of the path as the map name
			FString Name = Path;
			int32 SlashIdx;
			if (Path.FindLastChar(TEXT('/'), SlashIdx))
				Name = Path.RightChop(SlashIdx + 1);

			MapOptions.Add(MakeShared<FString>(Name));
		}
	}

	if (MapOptions.IsEmpty())
		MapOptions.Add(MakeShared<FString>(TEXT("(no maps in pool)")));

	SelectedMap = MapOptions[0];

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

				// Username label
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 4.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Username")))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
				]

				// Username input
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 24.f)
				[
					SAssignNew(UsernameInputBox, SEditableTextBox)
					.HintText(FText::FromString(TEXT("Enter your name...")))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
				]

				// Map label
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 4.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Map")))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
				]

				// Map dropdown
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 16.f)
				[
					SNew(SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&MapOptions)
					.OnSelectionChanged_Lambda([this](TSharedPtr<FString> Item, ESelectInfo::Type /*SelectInfo*/)
					{
						if (Item.IsValid())
							SelectedMap = Item;
					})
					.OnGenerateWidget_UObject(this, &ULab_MenuWidget::MakeMapOptionWidget)
					.InitiallySelectedItem(SelectedMap)
					[
						SNew(STextBlock)
						.Text_UObject(this, &ULab_MenuWidget::GetSelectedMapText)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
					]
				]

				// HOST button
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 12.f)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.OnClicked_UObject(this, &ULab_MenuWidget::OnHostClicked)
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
					.OnClicked_UObject(this, &ULab_MenuWidget::OnJoinClicked)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("JOIN GAME")))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
					]
				]
			]
		];
}

TSharedRef<SWidget> ULab_MenuWidget::MakeMapOptionWidget(TSharedPtr<FString> Item) const
{
	return SNew(STextBlock)
		.Text(FText::FromString(Item.IsValid() ? *Item : TEXT("")))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14));
}

FText ULab_MenuWidget::GetSelectedMapText() const
{
	return FText::FromString(SelectedMap.IsValid() ? *SelectedMap : TEXT(""));
}

void ULab_MenuWidget::SaveUsername() const
{
	if (!UsernameInputBox.IsValid()) return;

	FString Name = UsernameInputBox->GetText().ToString().TrimStartAndEnd();
	if (Name.IsEmpty()) Name = TEXT("Player");

	if (ULab_GameInstance* GI = GetGameInstance<ULab_GameInstance>())
		GI->SetLocalUsername(Name);
}

FReply ULab_MenuWidget::OnHostClicked()
{
	if (MapOptions.IsEmpty() || !SelectedMap.IsValid())
		return FReply::Handled();

	ULab_GameInstance* GI = GetGameInstance<ULab_GameInstance>();
	if (!GI) return FReply::Handled();

	// Resolve the display name back to a full map path from the pool
	FString MapPath;
	for (int32 i = 0; i < GI->MapPool.Num(); ++i)
	{
		FString Name = GI->MapPool[i];
		int32 SlashIdx;
		if (Name.FindLastChar(TEXT('/'), SlashIdx))
			Name = Name.RightChop(SlashIdx + 1);

		if (Name == *SelectedMap)
		{
			MapPath = GI->MapPool[i];
			break;
		}
	}

	if (MapPath.IsEmpty()) return FReply::Handled();

	SaveUsername();
	RemoveFromParent();
	GI->HostGame(MapPath, 3);

	return FReply::Handled();
}

FReply ULab_MenuWidget::OnJoinClicked()
{
	if (!IPInputBox.IsValid()) return FReply::Handled();

	const FString IP = IPInputBox->GetText().ToString().TrimStartAndEnd();
	if (IP.IsEmpty()) return FReply::Handled();

	SaveUsername();

	if (ULab_GameInstance* GI = GetGameInstance<ULab_GameInstance>())
	{
		RemoveFromParent();
		GI->JoinGameByIP(IP);
	}
	return FReply::Handled();
}
