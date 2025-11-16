#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ActorComponent.h"
#include "InputAction.h"
#include "EnhancedInputComponent.h"
#include "GrabberComponent.generated.h"

class UPhysicsHandleComponent;
class UInputComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NINETYNINEPINKBALLS_API UGrabberComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGrabberComponent();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* GrabThrowAction;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION()
	void GrabThrow();

	UFUNCTION()
	void Grab();

	UFUNCTION()
	void Throw();
	
	UFUNCTION()
	void ShowWinWidget();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	float ReachDistance = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	float ThrowStrength = 1500.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* WinningSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	TSubclassOf<UUserWidget> WinWidgetClass;

private:
	// Helpers
	void FindPhysicsHandle();
	void SetupInputComponent();
	FHitResult GetFirstPhysicsBodyInReach() const;
	void GetPlayerViewPoint(FVector& OutLocation, FRotator& OutRotation) const;

	void PlayWinningSound() const;
	
	// Owned components
	UPROPERTY()
	UPhysicsHandleComponent* PhysicsHandle = nullptr;

	UPROPERTY()
	UEnhancedInputComponent* EnhancedInputComponent = nullptr;
	
	UPROPERTY()
	AActor* GrabbedItem = nullptr;
};
