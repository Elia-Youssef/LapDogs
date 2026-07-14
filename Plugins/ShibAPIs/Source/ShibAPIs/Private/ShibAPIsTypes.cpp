// Copyright Shiba Inu Games LLC.


#include "ShibAPIsTypes.h"
#include "JsonObjectConverter.h"


TSharedPtr<FJsonObject> UShibAPIsTypes::ParseJsonAsObject(const FString& JsonData)
{
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonData);

    TSharedPtr<FJsonValue> JsonValue;
    if (!FJsonSerializer::Deserialize(Reader, JsonValue)) return nullptr;

    TSharedPtr<FJsonObject>* JsonObject;
    if (!JsonValue.IsValid() || !JsonValue->TryGetObject(JsonObject)) return nullptr;

	return *JsonObject;
}

TArray<TSharedPtr<FJsonValue>> UShibAPIsTypes::ParseJsonAsArray(const FString& JsonData)
{
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonData);

    TSharedPtr<FJsonValue> JsonValue;
    if (!FJsonSerializer::Deserialize(Reader, JsonValue)) return TArray<TSharedPtr<FJsonValue>>();

    TArray<TSharedPtr<FJsonValue>>* JsonArray;
    if (!JsonValue.IsValid() || !JsonValue->TryGetArray(JsonArray)) return TArray<TSharedPtr<FJsonValue>>();

	return *JsonArray;
}