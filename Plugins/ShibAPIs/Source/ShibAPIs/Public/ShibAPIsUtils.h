// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "ShibAPIsUtils.generated.h"

DECLARE_DELEGATE_ThreeParams(FHttpRequestCallback, bool, int32, const FString&);

UCLASS()
class SHIBAPIS_API UShibAPIsUtils : public UObject
{
	GENERATED_BODY()

private:
	/**
	 * @param Method `Get` | `Post` | `Patch` | `Put` | `Delete`
	 * @param Url Url to call
	 * @param Content Request body (Json)
	 * @param Callback Callback function
	 * @return was the request successful or not
	 */
	static bool HttpRequestInternal(const FString& Method, const FString& Url, const FString& Content,
	                                const FHttpRequestCallback& Callback,
	                                const TMap<FString, FString>& Headers = {});

public:
	/**
	 * Http Request with a struct body
	 */
	template <typename InStructType>
	static bool HttpRequest(const FString& Method, const FString& Url, const InStructType& Content,
	                        const FHttpRequestCallback& Callback, const TMap<FString, FString>& Headers = {})
	{
		FString ContentAsString = FString();
		if (Method != "Get")
		{
			ContentAsString = StructToJson(Content);
		}
		return HttpRequestInternal(Method, Url, ContentAsString, Callback, Headers);
	}

	/**
	 * Http Request with a string body
	 */
	static bool HttpRequest(const FString& Method, const FString& Url, const FString& Content,
	                        const FHttpRequestCallback& Callback, const TMap<FString, FString>& Headers = {})
	{
		return HttpRequestInternal(Method, Url, Content, Callback, Headers);
	}

	/**
	 * Http Request that has no body, generally `Get` requests
	 */
	static bool HttpRequest(const FString& Method, const FString& Url, const FHttpRequestCallback& Callback,
	                        const TMap<FString, FString>& Headers = {})
	{
		return HttpRequestInternal(Method, Url, FString(), Callback, Headers);
	}

	template <typename InStructType>
	static FString StructToJson(const InStructType& Struct)
	{
		FString OutString = FString();
		FJsonObjectConverter::UStructToJsonObjectString<InStructType>(Struct, OutString);
		return OutString;
	}

	template <typename InStructType>
	static FString StructToJson(const TArray<InStructType>& StructArray)
	{
		FString OutString = FString();

		// FString::Join modified
		bool bFirst = true;
		for (const auto& i : StructArray)
		{
			if (bFirst) { bFirst = false; }
			else { OutString += ","; }

			OutString += StructToJson(i);
		}

		return FString::Printf(TEXT("[%s]"), *OutString);
	}

	static FString StringArrayToJson(const TArray<FString>& StringArray)
	{
		FString OutString;
		
		// FString::Join modified
		bool bFirst = true;
		for (const auto& i : StringArray)
		{
			if (bFirst) { bFirst = false; }
			else { OutString += ","; }

			OutString += FString::Printf(TEXT("\"%s\""), *i);
		}
		
		return FString::Printf(TEXT("[%s]"), *OutString);
	}
};
