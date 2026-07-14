// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibAPIsTypes.h"
#include "ShibAPIsUtils.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ShibAPIsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResponse, bool, bSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetUserInfoDelegate, FGetUserInfoResponse, GetUserInfoResponse, bool, bSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetUserRegion, FUserRegion, Region, bool, bSuccessful);

/**
 * 
 */
UCLASS()
class SHIBAPIS_API UShibAPIsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

#pragma region HttpRequests
protected:
	/**
	 * This function calls `UShibAPIsUtils::HttpRequest`
	 * but calls `GetUrl` and `GetAuth` first, so no need to override it in every subsystem.
	 * To call custom URLs use `UShibAPIsUtils` directly.
	 */
	virtual bool HttpRequest(const FString& Method, const FString& Endpoint, const FString& Content = FString(),
	                         const FHttpRequestCallback& Callback = nullptr);
	/**
	 * This function returns the full URL to call given an endpoint.
	 * Pass an endpoint to append to the end of the base URL.
	 * Override this in other subsystems to change the base URL.
	 */
	virtual bool GetUrl(FString& Url);
	/**
	 * This function returns the authorization header value to pass to the `HttpRequest` headers.
	 * Override this in other subsystems to change the authorization.
	 */
	virtual TMap<FString, FString> GetAuth();
#pragma endregion HttpRequests

	// The time in between two updates of MyPlotsInfo variable.
	// This is used to limit the amount of calls we do to the API.
	// This variable represents minutes (3.f = 3 minutes)
	float MyPlotsInfoTimeLifespan = 3.f;

	// The last time the MyPlotsInfo variable got updated
	FDateTime MyPlotsInfoLastUpdateTime = FDateTime();

public:
	UPROPERTY(BlueprintReadOnly)
	FGetUserInfoResponse UserInfo;

	UPROPERTY(BlueprintReadOnly)
	FUserRegion UserRegion;

public:
	/**
	 * GET REGION
	 */
	FString GetUserRegionUrl = FString("https://ipapi.co/json");
	UFUNCTION(BlueprintCallable, Category = "Requests|Logs")
	void GetUserRegion();
	UFUNCTION()
	void OnGetUserRegion(bool bSuccessful, int32 Status, const FString& ResponseAsString);
	UPROPERTY(BlueprintAssignable, Category = "Requests|Logs|Delegates")
	FOnGetUserRegion OnGetUserRegionDelegate;

	/**
	 *  GET USER
	 */
	FString GetUserInfoEndpoint = FString("/api/USER/SHOWUSERINFO");
	UFUNCTION(BlueprintCallable, Category = "Requests|User")
	void GetUserInfo();
	UFUNCTION()
	void OnGetUserInfo(bool bSuccessful, int32 Status, const FString& ResponseAsString);
	UPROPERTY(BlueprintAssignable, Category = "Requests|User|Delegates")
	FOnGetUserInfoDelegate OnGetUserInfoDelegate;
};
