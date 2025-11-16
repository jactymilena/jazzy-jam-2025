#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Components/SphereComponent.h"
#include "PoloResponseComponent.generated.h"


UCLASS(Blueprintable, BlueprintType)
class NINETYNINEPINKBALLS_API UPoloResponseComponent : public USphereComponent
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float MyPoloCollisionRadius = 50.0f;
	
	UFUNCTION(BlueprintCallable)
	void RespondPolo();
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPoloResponseDelegate);

	UPROPERTY(BlueprintAssignable, Category = "Delegates")
	FPoloResponseDelegate OnPoloResponse;
};
