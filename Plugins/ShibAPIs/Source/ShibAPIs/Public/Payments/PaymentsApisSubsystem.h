// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibAPIsSubsystem.h"
#include "PaymentsApisTypes.h"
#include "PaymentsApisSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetBalance, const TArray<FBalance>&, Balance, bool, bSuccessful);

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnBankTransactionCreated, bool, bSuccessful);

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnCryptoTransactionCreated, const FCryptoTransactionDetails&, Details, bool,
                                   bSuccessful);

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnGetTransactionStatus, ETransactionStatus, TransactionStatus, bool, bSuccessful);

/**
 * 
 */
UCLASS()
class SHIBAPIS_API UPaymentsApisSubsystem : public UShibAPIsSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * GET BALANCE
	 */
	FString GetBalanceEndpoint = FString("/api/GETBALANCE"); // dummy api
	UFUNCTION(BlueprintCallable, Category = "Requests|Balance")
	void GetBalance();
	UPROPERTY(BlueprintAssignable, Category = "Requests|Balance|Delegates")
	FOnGetBalance OnGetBalanceDelegate;

	/**
	 * CREATE BANK TRANSACTION
	 */
	FString CreateBankTransactionEndpoint = FString("/api/CREATEBANKTRANSACTION"); // dummy api
	UFUNCTION(BlueprintCallable, Category = "Requests|Balance")
	void CreateBankTransaction(FOnBankTransactionCreated Event, const FTransactionRequest& Transaction);

	/**
	 * CREATE CRYPTO TRANSACTION
	 */
	FString CreateCryptoTransactionEndpoint = FString("/api/CREATECRYPTOTRANSACTION"); // dummy api
	UFUNCTION(BlueprintCallable, Category = "Requests|Balance")
	void CreateCryptoTransaction(FOnCryptoTransactionCreated Event, const FTransactionRequest& Transaction);

	/**
	 * GET TRANSACTION STATUS
	 */
	FString GetTransactionStatusEndpoint = FString("/api/GETTRANSACTIONSTATUS"); // dummy api
	UFUNCTION(BlueprintCallable, Category = "Requests|Balance")
	void GetTransactionStatus(FOnGetTransactionStatus Event, const FString& TransactionId);
};
