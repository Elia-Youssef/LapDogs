// Copyright Shiba Inu Games LLC.

#include "ShibAPIsSubsystem.h"

#include "ShibAPIsSettings.h"
#include "ShibAPIsUtils.h"


#pragma region HttpRequests
bool UShibAPIsSubsystem::HttpRequest(const FString& Method, const FString& Endpoint, const FString& Content,
                                     const FHttpRequestCallback& Callback)
{
	FString Url;
	if (!GetUrl(Url))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get URL, aborting request."));
		return false;
	}

	return UShibAPIsUtils::HttpRequest(Method, Url + Endpoint, Content, Callback, GetAuth());
}

bool UShibAPIsSubsystem::GetUrl(FString& Url)
{
	auto* Settings = UShibAPIsSettings::Get();
	if (!Settings || Settings->ServerURL.IsEmpty())
	{
		return false;
	}

	Url = Settings->ServerURL;

	return true;
}

TMap<FString, FString> UShibAPIsSubsystem::GetAuth()
{
	FString Token;
	FParse::Value(FCommandLine::Get(), *FString("token="), Token);
	return {{FString("Authorization"), FString::Printf(TEXT("Bearer %s"), *Token)}};
}
#pragma endregion HttpRequests

void UShibAPIsSubsystem::GetUserRegion()
{
	if (!UserRegion.ip.IsEmpty()) // if there's an ip, then we already have the region
	{
		OnGetUserRegionDelegate.Broadcast(UserRegion, true);
		return;
	}

	FHttpRequestCallback Callback;
	Callback.BindUObject(this, &ThisClass::OnGetUserRegion);
	UShibAPIsUtils::HttpRequest("Get", GetUserRegionUrl, Callback);
}

void UShibAPIsSubsystem::OnGetUserRegion(bool bSuccessful, int32 Status, const FString& ResponseAsString)
{
	if (!bSuccessful)
	{
		OnGetUserRegionDelegate.Broadcast(FUserRegion{}, false);
		return;
	}

	bool bParsed = FJsonObjectConverter::JsonObjectStringToUStruct<FUserRegion>(ResponseAsString, &UserRegion);
	UserRegion.FullJsonResponse = ResponseAsString;
	OnGetUserRegionDelegate.Broadcast(UserRegion, bParsed);
}

void UShibAPIsSubsystem::GetUserInfo()
{
	// TODO: REMOVE THIS TOKEN
	FString Token;
	FParse::Value(FCommandLine::Get(), *FString("token="), Token);
	FHttpRequestCallback Callback;
	Callback.BindUObject(this, &ThisClass::OnGetUserInfo);
	HttpRequest("Get", GetUserInfoEndpoint + "?Token=" + Token, FString(), Callback);
}

void UShibAPIsSubsystem::OnGetUserInfo(bool bSuccessful, int32 Status, const FString& ResponseAsString)
{
	// TODO: refactor this mess
	if (!bSuccessful)
	{
		OnGetUserInfoDelegate.Broadcast(FGetUserInfoResponse(), false);
		return;
	}

	FGetUserInfoResponse GetUserInfoResponse;
	auto JsonData = UShibAPIsTypes::ParseJsonAsObject(ResponseAsString);
	if (!JsonData)
	{
		OnGetUserInfoDelegate.Broadcast(FGetUserInfoResponse(), false);
		return;
	}

	if (JsonData->HasTypedField<EJson::Object>(TEXT("User")))
	{
		const TSharedPtr<FJsonObject> UserObject = JsonData->GetObjectField(TEXT("User"));
		FJsonObjectConverter::JsonObjectToUStruct<FShibUser>(UserObject.ToSharedRef(), &GetUserInfoResponse.User);
	}

	if (JsonData->HasTypedField<EJson::Array>(TEXT("Wallets")))
	{
		if (const TArray<TSharedPtr<FJsonValue>> WalletsObject = JsonData->GetArrayField(TEXT("Wallets")); WalletsObject
			.Num())
		{
			TSharedPtr<FJsonObject>* WalletObject;
			WalletsObject[0]->TryGetObject(WalletObject);
			FJsonObjectConverter::JsonObjectToUStruct<FShibWallet>(WalletObject->ToSharedRef(),
			                                                       &GetUserInfoResponse.Wallet);
		}
	}

	if (JsonData->HasTypedField<EJson::Array>(TEXT("NFTs")))
	{
		const TArray<TSharedPtr<FJsonValue>> NftsObject = JsonData->GetArrayField(TEXT("Wallets"));
		for (TSharedPtr<FJsonObject>* NftObject; const TSharedPtr<FJsonValue>& Nft : NftsObject)
		{
			Nft->TryGetObject(NftObject);
		}
	}

	UserInfo = GetUserInfoResponse;
	OnGetUserInfoDelegate.Broadcast(GetUserInfoResponse, bSuccessful);
}
