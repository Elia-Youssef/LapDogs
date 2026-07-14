// Fill out your copyright notice in the Description page of Project Settings.

#include "ShibUserEOS.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

UShibUserEOS::UShibUserEOS()
{
}

void UShibUserEOS::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	checkf(Subsystem != nullptr, TEXT("Unable to get Online Subsystem."));

	IOnlineIdentityPtr IdentityInterface = Subsystem->GetIdentityInterface();
	checkf(IdentityInterface != nullptr, TEXT("Unable to get Identity Interface."));
}

void UShibUserEOS::Deinitialize()
{
	Super::Deinitialize();
}

void UShibUserEOS::Login()
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineIdentityPtr IdentityInterface = Subsystem->GetIdentityInterface();
	checkf(IdentityInterface != nullptr, TEXT("Unable to get the Identity interface."));

	//Actually do login if the user is not
	if (IdentityInterface->GetLoginStatus(0) != ELoginStatus::LoggedIn)
	{
		LoginCompleteDelegateHandle = IdentityInterface->AddOnLoginCompleteDelegate_Handle(0, FOnLoginComplete::FDelegate::CreateUObject(this, &UShibUserEOS::HandleLoginComplete));

		// Auto login isn't that good, we should use login instead
		IdentityInterface->AutoLogin(0);

		/*
		// Maybe we will use this at some point
		FOnlineAccountCredentials Creds;
		Creds.Type = "Developer";
		Creds.Token = "ShibCreative";
		Creds.Id = "localhost:6300";
		IdentityInterface->Login(0, Creds);
		*/
	}
}

void UShibUserEOS::Logout()
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineIdentityPtr IdentityInterface = Subsystem->GetIdentityInterface();
	checkf(IdentityInterface != nullptr, TEXT("Unable to get the Identity interface."));

	LogoutCompleteDelegateHandle = IdentityInterface->AddOnLogoutCompleteDelegate_Handle(0, FOnLogoutComplete::FDelegate::CreateUObject(this, &UShibUserEOS::HandleLogoutComplete));

	IdentityInterface->Logout(0);
}

void UShibUserEOS::HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineIdentityPtr IdentityInterface = Subsystem->GetIdentityInterface();

	IdentityInterface->ClearOnLoginCompleteDelegate_Handle(0, LoginCompleteDelegateHandle);
	LoginCompleteDelegateHandle.Reset();

	if (bWasSuccessful)
	{
		TSharedPtr<FUserOnlineAccount> OnlineAccount = IdentityInterface->GetUserAccount(UserId);
		PlayerName = IdentityInterface->GetPlayerNickname(UserId);
		OnlineAccount->GetUserAttribute("externalAccount.epic.id", EpicAccountID);
		OnlineAccount->GetUserAttribute("productUserId", EpicProductUserID);
		OnlineAccount->GetAuthAttribute("authenticatedWith", AuthenticationSource);
		EOSConnectIDToken = IdentityInterface->GetAuthToken(LocalUserNum);
		NetID = FUniqueNetIdRepl(UserId);

		UE_LOG(LogTemp, Log, TEXT( "logged in successfully Player Name: %s"), *PlayerName);
		UE_LOG(LogTemp, Log, TEXT( "Epic Account ID is: %s"), *EpicAccountID);
		UE_LOG(LogTemp, Log, TEXT( "Product User ID is: %s"), *EpicProductUserID);
		UE_LOG(LogTemp, Log, TEXT( "was authenticated with: %s"), *AuthenticationSource);
		UE_LOG(LogTemp, Log, TEXT( "EOS Connect ID is: %s"), *EOSConnectIDToken);
		
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT( "Failed to complete EOS Login with an error of: %s"), *Error);

		if (Error.Contains("EOS_InvalidAuth"))
		{
			UE_LOG(LogTemp, Warning, TEXT( "Potentially failed because it attempted to login a user with persistent authentication when they aren't logged in yet"));
			Login();
		}
	}

	OnLoginComplete.Broadcast(bWasSuccessful, PlayerName);
}

void UShibUserEOS::HandleLogoutComplete(int LocalUserNum, bool bWasSuccessful)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineIdentityPtr IdentityInterface = Subsystem->GetIdentityInterface();

	IdentityInterface->ClearOnLogoutCompleteDelegate_Handle(0, LogoutCompleteDelegateHandle);
	LogoutCompleteDelegateHandle.Reset();

	OnLogoutComplete.Broadcast(bWasSuccessful);
}

bool UShibUserEOS::IsPlayerLoggedIn()
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineIdentityPtr IdentityInterface = Subsystem->GetIdentityInterface();
	checkf(IdentityInterface != nullptr, TEXT("Unable to get the Identity interface."));

	return IdentityInterface->GetLoginStatus(0) == ELoginStatus::LoggedIn;
}


