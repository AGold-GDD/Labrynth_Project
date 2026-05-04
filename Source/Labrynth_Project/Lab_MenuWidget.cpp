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
	// Cache username so it survives a Slate-triggered rebuild
	FString CachedUsername;
	if (UsernameInputBox.IsValid())
		CachedUsername = UsernameInputBox->GetText().ToString();

	// Only populate once — don't reset the user's selection on re-entry
	if (MapOptions.IsEmpty())
	{
		if (ULab_GameInstance* GI = GetGameInstance<ULab_GameInstance>())
		{
			for (const FString& Path : GI->MapPool)
			{
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
	}

	if (HostOptions.IsEmpty())
	{
		HostOptions.Add(MakeShared<FString>(TEXT("— type IP below —")));
		if (ULab_GameInstance* GI = GetGameInstance<ULab_GameInstance>())
		{
			for (const FKnownHost& Host : GI->KnownHosts)
				HostOptions.Add(MakeShared<FString>(FString::Printf(TEXT("%s  %s"), *Host.Name, *Host.IP)));
		}
		SelectedHost = HostOptions[0];
	}

	if (PlayerCountOptions.IsEmpty())
	{
		for (int32 i = 3; i <= 6; ++i)
			PlayerCountOptions.Add(MakeShared<FString>(FString::FromInt(i)));
		SelectedPlayerCount = PlayerCountOptions[0];
	}

	TSharedRef<SWidget> Result = SNew(SBox)
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
					.OnSelectionChanged_Lambda([this](TSharedPtr<FString> Item, ESelectInfo::Type)
					{
						if (Item.IsValid()) SelectedMap = Item;
					})
					.OnGenerateWidget_UObject(this, &ULab_MenuWidget::MakeMapOptionWidget)
					.InitiallySelectedItem(SelectedMap)
					[
						SNew(STextBlock)
						.Text_UObject(this, &ULab_MenuWidget::GetSelectedMapText)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
					]
				]

				// Player count label
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 4.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Players")))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
				]

				// Player count dropdown (3–6)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 16.f)
				[
					SNew(SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&PlayerCountOptions)
					.OnSelectionChanged_Lambda([this](TSharedPtr<FString> Item, ESelectInfo::Type)
					{
						if (Item.IsValid()) SelectedPlayerCount = Item;
					})
					.OnGenerateWidget_UObject(this, &ULab_MenuWidget::MakePlayerCountWidget)
					.InitiallySelectedItem(SelectedPlayerCount)
					[
						SNew(STextBlock)
						.Text_UObject(this, &ULab_MenuWidget::GetSelectedPlayerCountText)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
					]
				]

				// HOST button
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 24.f)
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

				// Quick connect label
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 4.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Quick Connect")))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
				]

				// Known hosts dropdown
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 8.f)
				[
					SNew(SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&HostOptions)
					.OnSelectionChanged_Lambda([this](TSharedPtr<FString> Item, ESelectInfo::Type)
					{
						if (!Item.IsValid() || !IPInputBox.IsValid()) return;
						// Parse the IP out of "Name  IP" — skip the placeholder entry
						FString Entry = *Item;
						int32 LastSpace = INDEX_NONE;
						for (int32 i = Entry.Len() - 1; i >= 0; --i)
						{
							if (Entry[i] == TEXT(' '))
							{
								LastSpace = i;
								break;
							}
						}
						if (LastSpace != INDEX_NONE)
							IPInputBox->SetText(FText::FromString(Entry.RightChop(LastSpace + 1)));
						SelectedHost = Item;
					})
					.OnGenerateWidget_UObject(this, &ULab_MenuWidget::MakeHostOptionWidget)
					.InitiallySelectedItem(SelectedHost)
					[
						SNew(STextBlock)
						.Text_UObject(this, &ULab_MenuWidget::GetSelectedHostText)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
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

	if (!CachedUsername.IsEmpty() && UsernameInputBox.IsValid())
		UsernameInputBox->SetText(FText::FromString(CachedUsername));

	return Result;
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

TSharedRef<SWidget> ULab_MenuWidget::MakeHostOptionWidget(TSharedPtr<FString> Item) const
{
	return SNew(STextBlock)
		.Text(FText::FromString(Item.IsValid() ? *Item : TEXT("")))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14));
}

FText ULab_MenuWidget::GetSelectedHostText() const
{
	return FText::FromString(SelectedHost.IsValid() ? *SelectedHost : TEXT(""));
}

TSharedRef<SWidget> ULab_MenuWidget::MakePlayerCountWidget(TSharedPtr<FString> Item) const
{
	return SNew(STextBlock)
		.Text(FText::FromString(Item.IsValid() ? *Item : TEXT("")))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14));
}

FText ULab_MenuWidget::GetSelectedPlayerCountText() const
{
	return FText::FromString(SelectedPlayerCount.IsValid() ? *SelectedPlayerCount : TEXT("3"));
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

	const int32 PlayerCount = SelectedPlayerCount.IsValid()
		? FMath::Clamp(FCString::Atoi(**SelectedPlayerCount), 3, 6) : 3;

	SaveUsername();
	RemoveFromParent();
	GI->HostGame(MapPath, PlayerCount);

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
