#include "Lab_NameplateWidget.h"
#include "Lab_PlayerState.h"
#include "GameFramework/Character.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"

void ULab_NameplateWidget::SetOwningCharacter(ACharacter* Char)
{
	OwningChar = Char;
}

TSharedRef<SWidget> ULab_NameplateWidget::RebuildWidget()
{
	TAttribute<FText> TextAttr;
	TextAttr.BindUObject(this, &ULab_NameplateWidget::GetNameText);

	return SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(TextAttr)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			.ColorAndOpacity(FLinearColor::White)
			.ShadowOffset(FVector2D(1.f, 1.f))
			.ShadowColorAndOpacity(FLinearColor::Black)
			.Justification(ETextJustify::Center)
		];
}

FText ULab_NameplateWidget::GetNameText() const
{
	if (OwningChar.IsValid())
	{
		if (const ALab_PlayerState* PS = OwningChar->GetPlayerState<ALab_PlayerState>())
		{
			if (!PS->DisplayName.IsEmpty())
				return FText::FromString(PS->DisplayName);
		}
	}
	return FText::GetEmpty();
}
