// Copyright Shiba Inu Games LLC.


#include "ShibAPIsUtils.h"

#include "HttpModule.h"
#include "ShibAPIsSettings.h"
#include "Interfaces/IHttpResponse.h"

bool UShibAPIsUtils::HttpRequestInternal(const FString& Method, const FString& Url, const FString& Content,
                                         const FHttpRequestCallback& Callback, const TMap<FString, FString>& Headers)
{
	if (Url.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%hs] - Empty Url"), __FUNCTION__);
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("[%hs] - Url: %s"), __FUNCTION__, *Url);
	UE_LOG(LogTemp, Log, TEXT("[%hs] - Content: %s"), __FUNCTION__, *Content);

	const TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetVerb(Method);

	HttpRequest->SetHeader("Content-Type", "application/json");
	for (auto& [Key, Value] : Headers)
	{
		HttpRequest->SetHeader(Key, Value);
	}

	HttpRequest->SetURL(Url);
	HttpRequest->SetContentAsString(Content);

	if (Callback.IsBound())
	{
		HttpRequest->OnProcessRequestComplete().BindLambda(
			[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccessful)
			{
				UE_LOG(LogTemp, Log, TEXT("[%hs] - Response is valid: %d"), __FUNCTION__, Response.IsValid());
				if (!Response.IsValid())
				{
					Callback.Execute(false, -1, FString());
					return;
				}

				UE_LOG(LogTemp, Log, TEXT("[%hs] - bSuccessful: %d\n\tStatus: %d\n\t"), __FUNCTION__, bSuccessful,
				       Response.Get()->GetStatus());
				Callback.Execute(bSuccessful, Response.Get()->GetStatus(), Response.Get()->GetContentAsString());
			});
	}

	UE_LOG(LogTemp, Log, TEXT("[%hs] - PROCESSING REQUEST"), __FUNCTION__);
	return HttpRequest->ProcessRequest();
}
