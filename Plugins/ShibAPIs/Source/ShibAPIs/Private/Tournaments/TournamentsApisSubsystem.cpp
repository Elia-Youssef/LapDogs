// Copyright Shiba Inu Games LLC.

#include "Tournaments/TournamentsApisSubsystem.h"

#include "ShibAPIsSettings.h"

#pragma region HttpRequests
bool UTournamentsApisSubsystem::GetUrl(FString& Url)
{
	auto* Settings = UShibAPIsSettings::Get();
	if (!Settings || Settings->TournamentsServerURL.IsEmpty())
	{
		return false;
	}

	Url = Settings->TournamentsServerURL;

	return true;
}

TMap<FString, FString> UTournamentsApisSubsystem::GetAuth()
{
	auto* Settings = UShibAPIsSettings::Get();
	if (!Settings || Settings->TournamentsServerApiKey.IsEmpty())
	{
		return {};
	}

	return {{FString("x-api-key"), Settings->TournamentsServerApiKey}};
}
#pragma endregion HttpRequests

void UTournamentsApisSubsystem::IsTournamentValid(FOnTournamentResponse Event, FString TournamentId)
{
	FHttpRequestCallback Callback;
	Callback.BindLambda([Event](bool bSuccessful, int32 Status, const FString& ResponseAsString)
	{
		if (!bSuccessful)
		{
			Event.Execute(false);
			return;
		}

		TSharedPtr<FJsonObject> Object = UShibAPIsTypes::ParseJsonAsObject(ResponseAsString);
		if (!Object)
		{
			Event.Execute(false);
			return;
		}

		Event.Execute(Object->GetBoolField(TEXT("valid")));
	});

	FTournamentAddressRequest TempRequestBody;
	TempRequestBody.address = TournamentId;
	HttpRequest("Post", IsTournamentValidEndpoint, UShibAPIsUtils::StructToJson(TempRequestBody), Callback);
}

void UTournamentsApisSubsystem::IsTournamentActive(FOnTournamentResponse Event, FString TournamentId)
{
	FHttpRequestCallback Callback;
	Callback.BindLambda([Event](bool bSuccessful, int32 Status, const FString& ResponseAsString)
	{
		if (!bSuccessful)
		{
			Event.Execute(false);
			return;
		}

		TSharedPtr<FJsonObject> Object = UShibAPIsTypes::ParseJsonAsObject(ResponseAsString);
		if (!Object)
		{
			Event.Execute(false);
			return;
		}
		Event.Execute(Object->GetBoolField(TEXT("active")));
	});

	FTournamentAddressRequest TempRequestBody;
	TempRequestBody.address = TournamentId;
	HttpRequest("Post", IsTournamentActiveEndpoint, UShibAPIsUtils::StructToJson(TempRequestBody), Callback);
}

void UTournamentsApisSubsystem::GetParticipants(FOnGetTournamentParticipants Event, FString TournamentId)
{
	FHttpRequestCallback Callback;
	Callback.BindLambda([Event](bool bSuccessful, int32 Status, const FString& ResponseAsString)
	{
		if (!bSuccessful)
		{
			Event.Execute({});
			return;
		}

		TArray<FTournamentPlayer> Participants;
		FJsonObjectConverter::JsonArrayStringToUStruct<FTournamentPlayer>(ResponseAsString, &Participants);
		Event.Execute(Participants);
	});

	FTournamentAddressRequest TempRequestBody;
	TempRequestBody.address = TournamentId;
	HttpRequest("Post", GetParticipantsEndpoint, UShibAPIsUtils::StructToJson(TempRequestBody), Callback);
}

void UTournamentsApisSubsystem::GetRewards(FOnGetTournamentRewards Event, FString TournamentId)
{
	FHttpRequestCallback Callback;
	Callback.BindLambda([Event](bool bSuccessful, int32 Status, const FString& ResponseAsString)
	{
		if (!bSuccessful)
		{
			Event.Execute({});
			return;
		}

		TArray<FTournamentRewards> Participants;
		FJsonObjectConverter::JsonArrayStringToUStruct<FTournamentRewards>(ResponseAsString, &Participants);
		Event.Execute(Participants);
	});

	FTournamentAddressRequest TempRequestBody;
	TempRequestBody.address = TournamentId;
	HttpRequest("Post", GetRewardsEndpoint, UShibAPIsUtils::StructToJson(TempRequestBody), Callback);
}

void UTournamentsApisSubsystem::SetPlayerScore(FOnTournamentResponse Event, FString TournamentId, FString Wallet,
                                               int32 Score)
{
	FHttpRequestCallback Callback;
	Callback.BindLambda([Event](bool bSuccessful, int32 Status, const FString& ResponseAsString)
	{
		if (!bSuccessful)
		{
			Event.Execute(false);
			return;
		}

		TSharedPtr<FJsonObject> Object = UShibAPIsTypes::ParseJsonAsObject(ResponseAsString);
		if (!Object)
		{
			Event.Execute(false);
			return;
		}
		Event.Execute(Object->GetBoolField(TEXT("success")));
	});

	FTournamentPlayerScoreRequest TempRequestBody;
	TempRequestBody.address = TournamentId;
	TempRequestBody.player = Wallet;
	TempRequestBody.score = Score;
	HttpRequest("Post", SetPlayerScoreEndpoint, UShibAPIsUtils::StructToJson(TempRequestBody), Callback);
}

void UTournamentsApisSubsystem::GetPlayerScore(FOnGetPlayerScore Event, FString TournamentId, FString Wallet)
{
	FHttpRequestCallback Callback;
	Callback.BindLambda([Event](bool bSuccessful, int32 Status, const FString& ResponseAsString)
	{
		if (!bSuccessful)
		{
			Event.Execute(FTournamentPlayer{});
			return;
		}

		FTournamentPlayer Player;
		FJsonObjectConverter::JsonObjectStringToUStruct<FTournamentPlayer>(ResponseAsString, &Player);
		Event.Execute(Player);
	});

	FTournamentPlayerRequest TempRequestBody;
	TempRequestBody.address = TournamentId;
	TempRequestBody.player = Wallet;
	HttpRequest("Post", GetPlayerScoreEndpoint, UShibAPIsUtils::StructToJson(TempRequestBody), Callback);
}

void UTournamentsApisSubsystem::GetTournaments(FOnGetTournaments Event)
{
	FHttpRequestCallback Callback;
	Callback.BindLambda([Event](bool bSuccessful, int32 Status, const FString& ResponseAsString)
	{
		if (!bSuccessful)
		{
			Event.Execute({});
			return;
		}

		TArray<FTournamentDetails> Tournaments = ParseTournamentsMapFromJson(ResponseAsString);
		Event.Execute(Tournaments);
	});

	HttpRequest("Post", GetTournamentsEndpoint, FString(), Callback);
}

void UTournamentsApisSubsystem::GetTournamentsByPlayer(FOnGetTournaments Event, FString Wallet)
{
	FHttpRequestCallback Callback;
	Callback.BindLambda([Event](bool bSuccessful, int32 Status, const FString& ResponseAsString)
	{
		if (!bSuccessful)
		{
			Event.Execute({});
			return;
		}

		TArray<FTournamentDetails> Tournaments = ParseTournamentsMapFromJson(ResponseAsString);
		Event.Execute(Tournaments);
	});

	FPlayerAddressRequest TempRequestBody;
	TempRequestBody.player = Wallet;
	HttpRequest("Post", GetTournamentsByPlayerEndpoint, UShibAPIsUtils::StructToJson(TempRequestBody), Callback);
}

TArray<FTournamentDetails> UTournamentsApisSubsystem::ParseTournamentsMapFromJson(const FString& Json)
{
	TArray<FTournamentDetails> Tournaments;

	TSharedPtr<FJsonObject> JsonObject = UShibAPIsTypes::ParseJsonAsObject(Json);
	if (!JsonObject)
	{
		return Tournaments;
	}

	for (auto [Key, Value] : JsonObject->Values)
	{
		FTournamentDetails ValueAsStruct;
		FJsonObjectConverter::JsonObjectToUStruct<FTournamentDetails>(Value->AsObject().ToSharedRef(), &ValueAsStruct);
		ValueAsStruct.TournamentId = Key;

		Tournaments.Add(ValueAsStruct);
	}

	return Tournaments;
}
