#include "MarcoCallComponent.h"
#include "PoloResponseComponent.h"

#include "Kismet/GameplayStatics.h"

void UMarcoCallComponent::PlayCallSound()
{
	if (IsValid(CallSound))
	{
		FVector SoundLocation = GetComponentLocation();
		FRotator SoundRotation = GetComponentRotation();
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			CallSound,
			SoundLocation,
			SoundRotation,
			VolumeMultiplier,
			PitchMultiplier,
			0.0f,
			AttenuationSettings
		);
	}
}

void UMarcoCallComponent::CallMarco()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Call Marco");
	TArray<AActor*> OverlappingBalls;
	GetOverlappingActors(OverlappingBalls);
	
	PlayCallSound();
	for (AActor* Actor : OverlappingBalls)
	{
		UPoloResponseComponent* PoloComponent = Actor->FindComponentByClass<UPoloResponseComponent>();
		if (IsValid(PoloComponent))
			PoloComponent->RespondPolo();
	}
}



