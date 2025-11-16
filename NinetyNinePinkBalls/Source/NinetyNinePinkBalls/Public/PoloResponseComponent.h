#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Components/SphereComponent.h"
#include "PoloResponseComponent.generated.h"


UCLASS(Blueprintable, BlueprintType)
class NINETYNINEPINKBALLS_API UPoloResponseComponent : public USphereComponent
{
	GENERATED_BODY()

private:
	void PlayResponseSound();	

public:	
	FTimerHandle MyDelayTimerHandle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	float DelayToRespond = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* ResponseSound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundAttenuation* AttenuationSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	float VolumeMultiplier = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	float PitchMultiplier = 1.0f;
	
	UFUNCTION(BlueprintCallable)
	void RespondPolo();
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPoloResponseDelegate);

	UPROPERTY(BlueprintAssignable, Category = "Delegates")
	FPoloResponseDelegate OnPoloResponse;
};
