// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseComponent.h"

#include "EnhancedInputComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UPauseComponent::UPauseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UPauseComponent::BeginPlay()
{
	Super::BeginPlay();
	SetupInput();
}

void UPauseComponent::Pause()
{
	if (!PauseWidgetClass) return;

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController) return;

	UUserWidget* pauseWidget = CreateWidget<UUserWidget>(PlayerController, PauseWidgetClass);
	if (IsValid(pauseWidget))
	{
		pauseWidget->AddToViewport();
		PlayerController->SetShowMouseCursor(true);
		PlayerController->SetInputMode(FInputModeUIOnly());
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.0f);
	}
}

void UPauseComponent::SetupInput()
{
	UEnhancedInputComponent* EnhancedInputComponent = GetOwner()->FindComponentByClass<UEnhancedInputComponent>();
	if (IsValid(EnhancedInputComponent))
	{
		EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &ThisClass::Pause);
	}
}
