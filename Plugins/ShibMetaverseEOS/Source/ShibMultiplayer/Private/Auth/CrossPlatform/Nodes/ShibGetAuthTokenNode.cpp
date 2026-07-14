
#if EOS_HAS_AUTHENTICATION

#include "Auth/CrossPlatform/Nodes/ShibGetAuthTokenNode.h"

#include "Interfaces/IHttpResponse.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "ShibMultiplayerSettings.h"
#include "Auth/AuthenticationHelpersShib.h"

namespace Redpoint::EOS::Auth::CrossPlatform
{

void FShibGetAuthTokenNode::Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone)
{
    auto* Settings = GetMutableDefault<UShibMultiplayerSettings>();
    // if (!Settings || Settings->ServerAuthenticationURL.IsEmpty())
    // {
    //     State->ErrorMessages.Add(TEXT("Invalid shib multiplayer server settings."));
    //     OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
    //     return;
    // }
    //
    // if (State->GetWorld())
    // {
    //     FTimerDelegate RequestDelegate;
    //     RequestDelegate.BindSP(this, &FShibGetAuthTokenNode::SendHttpRequest, Settings->ServerAuthenticationURL, State, OnDone);
    //     State->GetWorld()->GetTimerManager().SetTimer(RequestHandle, RequestDelegate, Settings->ServerRequestsRate, true);
    //     return;
    // }
    
    State->ErrorMessages.Add(TEXT("Invalid world."));
    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
}

void FShibGetAuthTokenNode::SendHttpRequest(FString URL, TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    UE_LOG(LogTemp, Warning, TEXT("Sending request again..."))

    auto HttpRequest = FHttpModule::Get().CreateRequest();

    HttpRequest->SetVerb("POST");
    HttpRequest->SetHeader("Content-Type", "application/json");
    HttpRequest->SetURL(URL);
    HttpRequest->SetContentAsString(FString::Printf(TEXT("{\"email\":\"nadim.shahoud@shib.io\",\"password\":\"test\"}")));
    HttpRequest->OnProcessRequestComplete().BindSP(this, &FShibGetAuthTokenNode::OnHttpResponse, State, OnDone);
    if (!HttpRequest->ProcessRequest())
    {
        State->ErrorMessages.Add(TEXT("Unable to start login request."));
        RequestHandle.Invalidate();
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
    }
}

void FShibGetAuthTokenNode::OnHttpResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone)
{
    if (!bConnectedSuccessfully)
    {
        State->ErrorMessages.Add(TEXT("Unable to connect to login URL for authentication."));
        RequestHandle.Invalidate();
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    if (Response->GetResponseCode() != EHttpResponseCodes::Ok)
    {
        State->ErrorMessages.Add(TEXT("Non-200 response from login URL when obtaining JWT for authentication."));
        RequestHandle.Invalidate();
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    // if (FAuthenticationHelpersShib::ParseLoginDataFromJSON(Response->GetContentAsString(), State)) {
    //     if (State->Metadata.Contains("SHIB_LOGGED_IN") && State->Metadata["SHIB_LOGGED_IN"])
    //     {
    //         RequestHandle.Invalidate();
    //         OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
    //         return;
    //     }
    //     return;
    // }

    RequestHandle.Invalidate();
    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
}

}

#endif
