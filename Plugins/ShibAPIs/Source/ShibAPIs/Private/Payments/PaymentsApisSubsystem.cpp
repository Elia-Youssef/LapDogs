// Copyright Shiba Inu Games LLC.

#include "Payments/PaymentsApisSubsystem.h"

void UPaymentsApisSubsystem::GetBalance()
{
	FHttpRequestCallback Callback;
	Callback.BindLambda([this](bool bSuccessful, int32 Status, const FString& ResponseAsString)
	{
		if (!bSuccessful)
		{
			OnGetBalanceDelegate.Broadcast({}, false);
			return;
		}

		TArray<FBalance> Balance;
		const bool bParsed = FJsonObjectConverter::JsonArrayStringToUStruct<FBalance>(ResponseAsString, &Balance);
		OnGetBalanceDelegate.Broadcast(Balance, bParsed);
	});

	HttpRequest("Get", GetBalanceEndpoint, FString(), Callback);
}

void UPaymentsApisSubsystem::CreateBankTransaction(FOnBankTransactionCreated Event,
                                                   const FTransactionRequest& Transaction)
{
	FHttpRequestCallback Callback;
	Callback.BindLambda([Event](bool bSuccessful, int32 Status, const FString& ResponseAsString)
	{
		Event.Execute(bSuccessful);
	});

	HttpRequest("Get", CreateBankTransactionEndpoint, FString(), Callback);
}

void UPaymentsApisSubsystem::CreateCryptoTransaction(FOnCryptoTransactionCreated Event,
                                                     const FTransactionRequest& Transaction)
{
	FHttpRequestCallback Callback;
	Callback.BindLambda([Event](bool bSuccessful, int32 Status, const FString& ResponseAsString)
	{
		if (!bSuccessful)
		{
			Event.Execute(FCryptoTransactionDetails{}, false);
			return;
		}

		FCryptoTransactionDetails Details;
		const bool bParsed = FJsonObjectConverter::JsonObjectStringToUStruct<FCryptoTransactionDetails>(
			ResponseAsString, &Details);
		Event.Execute(Details, bParsed);
	});

	HttpRequest("Get", CreateCryptoTransactionEndpoint, FString(), Callback);
}

void UPaymentsApisSubsystem::GetTransactionStatus(FOnGetTransactionStatus Event, const FString& TransactionId)
{
	FHttpRequestCallback Callback;
	Callback.BindLambda([Event](bool bSuccessful, int32 Status, const FString& ResponseAsString)
	{
		if (!bSuccessful)
		{
			Event.Execute(ETransactionStatus::TS_None, false);
			return;
		}

		FTransactionStatus Res;
		const bool bParsed = FJsonObjectConverter::JsonObjectStringToUStruct<
			FTransactionStatus>(ResponseAsString, &Res);
		Event.Execute(Res.Status, bParsed);
	});

	HttpRequest("Get", GetTransactionStatusEndpoint, FString(), Callback);
}
