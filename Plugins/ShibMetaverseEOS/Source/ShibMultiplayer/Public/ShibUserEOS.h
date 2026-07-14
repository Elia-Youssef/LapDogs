// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ShibUserEOS.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FShibLoginCompleteDelegateEOS, bool, bWasSuccessful, const FString, User);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FShibLogoutCompleteDelegateEOS, bool, bWasSuccessful);

/**
 * 
 */
UCLASS()
class SHIBMULTIPLAYER_API UShibUserEOS : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	public:

	explicit UShibUserEOS();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Shib Subsystems|User")
	void Login();

	UFUNCTION(BlueprintCallable, Category = "Shib Subsystems|User")
	void Logout();

	UFUNCTION(BlueprintPure, Category = "Shib Subsystems|User")
	bool IsPlayerLoggedIn();

	UFUNCTION(BlueprintGetter)
	inline FString GetPlayerName() const
	{
		return PlayerName;
	};

	UFUNCTION(BlueprintGetter)
	inline FString GetAuthenticationSource() const
	{
		return AuthenticationSource;
	};

	UFUNCTION(BlueprintGetter)
	inline FString GetEpicAccountID() const
	{
		return EpicAccountID;
	};

	UFUNCTION(BlueprintGetter)
	inline FString GetEpicProductUserID() const
	{
		return EpicProductUserID;
	};

	UFUNCTION(BlueprintGetter)
	inline FString GetEOSConnectIDToken() const
	{
		return EOSConnectIDToken;
	};

	UFUNCTION(BlueprintGetter)
	inline FUniqueNetIdRepl GetNetID() const
	{
		return NetID;
	};

	UPROPERTY(BlueprintAssignable, Category = "Shib Subsystems|User|Callback")
	FShibLoginCompleteDelegateEOS OnLoginComplete;

	UPROPERTY(BlueprintAssignable, Category = "Shib Subsystems|User|Callback")
	FShibLogoutCompleteDelegateEOS OnLogoutComplete;

private:
	FDelegateHandle LoginCompleteDelegateHandle;
	FDelegateHandle LogoutCompleteDelegateHandle;

	void HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	void HandleLogoutComplete(int LocalUserNum, bool bWasSuccessful);

	UPROPERTY(BlueprintReadOnly, BlueprintGetter = GetPlayerName, meta = (AllowPrivateAccess),Category="Shib User Subsystem")
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly, BlueprintGetter = GetAuthenticationSource, meta = (AllowPrivateAccess),Category="Shib User Subsystem")
	FString AuthenticationSource;

	UPROPERTY(BlueprintReadOnly, BlueprintGetter = GetEpicAccountID, meta = (AllowPrivateAccess),Category="Shib User Subsystem")
	FString EpicAccountID;

	UPROPERTY(BlueprintReadOnly, BlueprintGetter = GetEpicProductUserID, meta = (AllowPrivateAccess),Category="Shib User Subsystem")
	FString EpicProductUserID;

	UPROPERTY(BlueprintReadOnly, BlueprintGetter = GetEOSConnectIDToken, meta = (AllowPrivateAccess),Category="Shib User Subsystem")
	FString EOSConnectIDToken;

	UPROPERTY(BlueprintReadOnly, BlueprintGetter = GetNetID, meta = (AllowPrivateAccess),Category="Shib User Subsystem")
	FUniqueNetIdRepl NetID;
};
