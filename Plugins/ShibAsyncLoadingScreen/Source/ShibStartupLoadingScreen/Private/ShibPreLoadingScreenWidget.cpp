// Copyright Shiba Inu Games LLC.

#include "ShibPreLoadingScreenWidget.h"
#include "Widgets/Layout/SBorder.h"

void SShibPreLoadingScreenWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
[
	SNew(SBorder)
	.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
	.BorderBackgroundColor(FLinearColor::Black)
	.Padding(0)
];
}

void SShibPreLoadingScreenWidget::AddReferencedObjects(FReferenceCollector& Collector)
{
	//WidgetAssets.AddReferencedObjects(Collector);
}

FString SShibPreLoadingScreenWidget::GetReferencerName() const
{
	return TEXT("SShibPreLoadingScreenWidget");
}
