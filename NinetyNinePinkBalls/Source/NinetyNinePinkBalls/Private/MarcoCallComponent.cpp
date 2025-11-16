#include "MarcoCallComponent.h"
#include "PoloResponseComponent.h"
#include "Fonts/UnicodeBlockRange.h"

#include "Kismet/GameplayStatics.h"

void UMarcoCallComponent::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), "CanBeCaptured", FoundActors);
	
	if (FoundActors.Num() > 0 && IsValid(FoundActors[0]))
	{
		ActorToCall = FoundActors[0];
		PoloComponentToCall = ActorToCall->FindComponentByClass<UPoloResponseComponent>();
	}
}

void UMarcoCallComponent::PlayCallSound() const 
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
	
	if (PoloComponentToCall.IsValid())
	{
		PoloComponentToCall->RespondPolo();
	}
	
	for (const AActor* Actor : OverlappingBalls)
	{
		if (Actor != ActorToCall)
		{
			if (UPoloResponseComponent* PoloComponent = Actor->FindComponentByClass<UPoloResponseComponent>(); IsValid(PoloComponent))
			{
				PoloComponent->RespondPolo();
			}
		}
	}
}



