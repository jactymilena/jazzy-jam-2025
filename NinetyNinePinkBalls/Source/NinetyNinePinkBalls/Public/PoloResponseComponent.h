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
	float CalculateRandomDelay() const;
	bool ShouldRespond() const;
public:	
	FTimerHandle MyDelayTimerHandle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	float DelayToRespond = 2.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	float DelayToRespondDelta = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (UIMin = "0.0", UIMax = "100.0", ClampMin = "0.0", ClampMax = "100.0"))
	float ProbabilityToRespond = 100;
	
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
