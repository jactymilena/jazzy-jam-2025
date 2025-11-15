#include "MarcoCallComponent.h"
#include "PoloResponseComponent.h"

void UMarcoCallComponent::CallMarco()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Call Marco");
	TArray<AActor*> OverlappingBalls;
	GetOverlappingActors(OverlappingBalls);
	
	for (AActor* Actor : OverlappingBalls)
	{
		UPoloResponseComponent* PoloComponent = Actor->FindComponentByClass<UPoloResponseComponent>();
		if (IsValid(PoloComponent))
			PoloComponent->RespondPolo();
	}
	
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Array Count: %d"), OverlappingBalls.Num()));
}



