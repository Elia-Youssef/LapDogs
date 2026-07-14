// Copyright Shiba Inu Games LLC.

#include "LapDogs/LapDogsApisSubsystem.h"


void ULapDogsApisSubsystem::Log(const FShibLog& Log)
{
	HttpRequest("Post", LdLoggingEndpoint, UShibAPIsUtils::StructToJson(Log));
}

void ULapDogsApisSubsystem::SendGameStats(FSendGameStatsRequest SendGameStatsRequest)
{
	FHttpRequestCallback Callback;
	Callback.BindUObject(this, &ThisClass::OnSendGameStats);
	HttpRequest("Post", SendGameStatsEndpoint, UShibAPIsUtils::StructToJson(SendGameStatsRequest), Callback);
}

void ULapDogsApisSubsystem::OnSendGameStats(bool bSuccessful, int32 Status, const FString& ResponseAsString)
{
	OnSendGameStatsDelegate.Broadcast(bSuccessful);
}

void ULapDogsApisSubsystem::GetGameStats()
{
	// TODO: REMOVE THE USER ID FROM THIS
	FHttpRequestCallback Callback;
	Callback.BindUObject(this, &ThisClass::OnGetGameStats);
	HttpRequest("Get", GetGameStatsEndpoint + "?UserId=" + FString::FromInt(UserInfo.User.Id), FString(), Callback);
}

void ULapDogsApisSubsystem::OnGetGameStats(bool bSuccessful, int32 Status, const FString& ResponseAsString)
{
	if (!bSuccessful)
	{
		OnGetGameStatsDelegate.Broadcast(FGetGameStatsResponse(), false);
		return;
	}

	FGetGameStatsResponse GetGameStatsResponse;
	auto JsonData = UShibAPIsTypes::ParseJsonAsObject(ResponseAsString);
	if (!JsonData)
	{
		OnGetGameStatsDelegate.Broadcast(FGetGameStatsResponse(), false);
		return;
	}

	if (JsonData->HasTypedField<EJson::Object>(TEXT("Player")))
	{
		const TSharedPtr<FJsonObject> PlayerObject = JsonData->GetObjectField(TEXT("Player"));
		FJsonObjectConverter::JsonObjectToUStruct<FShibStatsPlayer>(PlayerObject.ToSharedRef(),
		                                                            &GetGameStatsResponse.Player);
	}

	if (JsonData->HasTypedField<EJson::Object>(TEXT("Stats")))
	{
		const TSharedPtr<FJsonObject> StatsObject = JsonData->GetObjectField(TEXT("Stats"));
		FJsonObjectConverter::JsonObjectToUStruct<FShibStats>(StatsObject.ToSharedRef(), &GetGameStatsResponse.Stats);
	}

	if (JsonData->HasTypedField<EJson::Array>(TEXT("Top3Records")))
	{
		const TArray<TSharedPtr<FJsonValue>> TopRecords = JsonData->GetArrayField(TEXT("Top3Records"));
		for (TSharedPtr<FJsonObject>* RecordObject; const TSharedPtr<FJsonValue>& Record : TopRecords)
		{
			Record->TryGetObject(RecordObject);

			FShibStatsRecords StatsRecord;
			FJsonObjectConverter::JsonObjectToUStruct<FShibStatsRecords>(RecordObject->ToSharedRef(), &StatsRecord);

			GetGameStatsResponse.TopRecords.Add(StatsRecord);
		}
	}

	OnGetGameStatsDelegate.Broadcast(GetGameStatsResponse, bSuccessful);
}
