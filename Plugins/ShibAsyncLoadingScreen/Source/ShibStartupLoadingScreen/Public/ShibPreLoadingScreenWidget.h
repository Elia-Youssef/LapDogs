// Copyright Shiba Inu Games LLC.

#pragma once

#include "UObject/GCObject.h"
#include "Widgets/Accessibility/SlateWidgetAccessibleTypes.h"
#include "Widgets/SCompoundWidget.h"

class FReferenceCollector;

class SHIBSTARTUPLOADINGSCREEN_API SShibPreLoadingScreenWidget : public SCompoundWidget, public FGCObject
{
public:
	SLATE_BEGIN_ARGS(SShibPreLoadingScreenWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	//~ Begin FGCObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;
	//~ End FGCObject interface
};
