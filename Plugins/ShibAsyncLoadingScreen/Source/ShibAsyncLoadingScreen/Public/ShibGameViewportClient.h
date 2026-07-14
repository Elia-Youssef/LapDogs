// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "ShibGameViewportClient.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class SHIBASYNCLOADINGSCREEN_API UShibGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()
public:

	/**
	 * Called after rendering the player views and HUDs to render menus, the console, etc.
	 * This is the last rendering call in the render loop
	 *
	 * @param Canvas        The canvas to use for rendering.
	 */
	virtual void PostRender( UCanvas* Canvas ) override;

	/** Clear fading state */
	virtual void ClearFade();

	/** Used for Fade to and from black */
	UFUNCTION(BlueprintCallable, Category=ShibLoadingScreen)
	virtual void Fade(const float WarmupDuration, const float Duration, const bool ToBlack);

	/** Does the actual screen fading */
	void DrawScreenFade(UCanvas* Canvas);

private:

	// Values used by our screen fading
	uint32 bFading : 1;
	uint32 bToBlack : 1; // Fading to black will be applied even if alpha is 1
	float FadeAlpha;
	float TimeElapsed;
	float FadeDuration;
	float FadeWarmupTime;
};
