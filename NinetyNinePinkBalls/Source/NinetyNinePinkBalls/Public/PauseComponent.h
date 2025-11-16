// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Components/ActorComponent.h"
#include "PauseComponent.generated.h"


UCLASS(Blueprintable, Abstract, meta=(BlueprintSpawnableComponent))
class NINETYNINEPINKBALLS_API UPauseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPauseComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	void SetupInput();
	void Pause();
	
	UPROPERTY(EditDefaultsOnly)
	const UInputAction* PauseAction = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> PauseWidgetClass = nullptr;
};
