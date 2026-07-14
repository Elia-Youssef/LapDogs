// Copyright Shiba Inu Games LLC.


#include "Inputs/ShibInputConfig.h"
#include "GameplayTagContainer.h"
#include "EnhancedInput/Public/InputAction.h"

const UInputAction* UShibInputConfig::FindInputActionForTag(const FGameplayTag& InputTag) const
{
	for (const FBHTaggedInputAction& TaggedInputAction : TaggedInputActions)
	{
		if (TaggedInputAction.InputAction && TaggedInputAction.InputTag == InputTag)
		{
			return TaggedInputAction.InputAction;
		}
	}

	return nullptr;
}
