// Copyright Shiba Inu Games LLC.

#include "Game/ShibGameSession.h"
#include "Player/ShibBaseController.h"

bool AShibGameSession::KickPlayer(APlayerController* KickedPlayer, const FText& KickReason)
{
	UNetConnection* Connection = Cast<UNetConnection>(KickedPlayer->Player);
	
	// Do not kick logged admins
	if (KickedPlayer != NULL && Connection != NULL)
	{
		if (KickedPlayer->GetPawn() != NULL)
		{
			KickedPlayer->PawnLeavingGame();
		}
		
		if (auto ShibPC = Cast<AShibBaseController>(KickedPlayer))
		{
			ShibPC->ClientWasKicked(KickReason);
		}
		
		if (KickedPlayer != NULL)
		{
			KickedPlayer->Destroy();
		}

		// Keeping this here but I don't think it is necessary
		//if (Connection->GetConnectionState() != USOCK_Closed)
		//{
		//	// Close player connection
		//	Connection->FlushNet(true);
		//	Connection->Close();
		//}

		return true;
	}
	return false;
}
