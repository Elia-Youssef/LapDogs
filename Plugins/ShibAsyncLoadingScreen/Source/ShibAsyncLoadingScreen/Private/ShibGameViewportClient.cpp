// Copyright Shiba Inu Games LLC.

#include "ShibGameViewportClient.h"
#include "Engine/Canvas.h"

DECLARE_LOG_CATEGORY_EXTERN(ShibLogGameViewport, Log, All);
DEFINE_LOG_CATEGORY(ShibLogGameViewport);

void UShibGameViewportClient::PostRender(UCanvas* Canvas)
{
	Super::PostRender(Canvas);
	
	// Fade if requested, you could use the same DrawScreenFade method from any canvas such as the HUD
	if (bFading)
	{
		DrawScreenFade(Canvas);
	}
}

void UShibGameViewportClient::ClearFade()
{
	UE_LOG(ShibLogGameViewport, Log, TEXT("Stop fading"));
	bFading = false;	
}

void UShibGameViewportClient::Fade(const float WarmupDuration, const float Duration, const bool ToBlack)
{
	if (World)
	{
		//UE_LOG(ShibLogGameViewport, Log, TEXT("Start fading - Warmup duration: %.2f, Duration: %.2f, To black: %d"), WarmupDuration, Duration, ToBlack ? 1 : 0);
		bFading = true;
		bToBlack = ToBlack;
		FadeWarmupTime = WarmupDuration;
		FadeDuration = Duration;
		TimeElapsed = 0.f;
	}	
}

void UShibGameViewportClient::DrawScreenFade(UCanvas* Canvas)
{
	if (World)
	{
		float FadeColorAlpha = bToBlack ? 1.f : 0.f;
		if (FadeWarmupTime > 0.0f)
		{
			FadeWarmupTime -= World->GetDeltaSeconds();
			//UE_LOG(ShibLogGameViewport, Log, TEXT("Currently warming up: WarmupTime=%.2f"),FadeWarmupTime);
			FadeColorAlpha = bToBlack ? 0.f : 1.f;
		}
		else
		{
			if (TimeElapsed < FadeDuration)
			{
				FadeColorAlpha = FMath::Lerp(bToBlack ? 0.f : 1.f, bToBlack ? 1.f : 0.f, TimeElapsed / FadeDuration);
				TimeElapsed += World->GetDeltaSeconds();
				//UE_LOG(ShibLogGameViewport, Log, TEXT("Currently fading: FadingTime=%.2f, Alpha=%.2f"),TimeElapsed, FadeColorAlpha);
			}
			
			// Make sure that we stay black in a fade to black
			if (!bToBlack && FadeColorAlpha == 0.f)
			{
				ClearFade();
			}
		}
		
		FColor OldColor = Canvas->DrawColor;
		FLinearColor FadeColor = FLinearColor::Black;
		FadeColor.A = FadeColorAlpha;
		Canvas->DrawColor = FadeColor.ToFColor(true);
		Canvas->DrawTile(Canvas->DefaultTexture, 0, 0, Canvas->ClipX, Canvas->ClipY, 0, 0, Canvas->DefaultTexture->GetSizeX(), Canvas->DefaultTexture->GetSizeY());
		Canvas->DrawColor = OldColor;
	}
}	