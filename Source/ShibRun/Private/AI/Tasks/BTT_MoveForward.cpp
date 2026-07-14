#include "AI/Tasks/BTT_MoveForward.h"

#include "Character/ShibCharacter.h"

EBTNodeResult::Type UBTT_MoveForward::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	
	ThisCharacter->AddMovementInput(ThisCharacter->GetActorForwardVector(), 1, true);
	
	return EBTNodeResult::Type::Succeeded;
}
