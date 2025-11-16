#include "GrabberComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"

UGrabberComponent::UGrabberComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGrabberComponent::BeginPlay()
{
	Super::BeginPlay();

	FindPhysicsHandle();
	SetupInputComponent();
}

void UGrabberComponent::FindPhysicsHandle()
{
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (!PhysicsHandle)
	{
		PhysicsHandle = NewObject<UPhysicsHandleComponent>(GetOwner(), UPhysicsHandleComponent::StaticClass(), TEXT("PhysicsHandle"));
		if (PhysicsHandle)
		{
			PhysicsHandle->RegisterComponent();
		}
	}
}

void UGrabberComponent::SetupInputComponent()
{
	EnhancedInputComponent = GetOwner()->FindComponentByClass<UEnhancedInputComponent>();
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(GrabThrowAction, ETriggerEvent::Started, this, &UGrabberComponent::GrabThrow);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s missing EnhancedInputComponent (grab won't work)."), *GetOwner()->GetName());
	}
}

void UGrabberComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PhysicsHandle) return;

	if (PhysicsHandle->GetGrabbedComponent())
	{
		// Keep the grabbed object at the reach point every tick
		FVector PlayerViewLocation;
		FRotator PlayerViewRotation;
		GetPlayerViewPoint(PlayerViewLocation, PlayerViewRotation);

		const FVector TargetLocation = PlayerViewLocation + PlayerViewRotation.Vector() * ReachDistance;
		PhysicsHandle->SetTargetLocationAndRotation(TargetLocation, PlayerViewRotation);
	}
}

FHitResult UGrabberComponent::GetFirstPhysicsBodyInReach() const
{
	FVector PlayerViewLocation;
	FRotator PlayerViewRotation;
	GetPlayerViewPoint(PlayerViewLocation, PlayerViewRotation);

	const FVector LineTraceEnd = PlayerViewLocation + PlayerViewRotation.Vector() * ReachDistance;

	FCollisionQueryParams TraceParams(FName(TEXT("")), false, GetOwner());
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FHitResult Hit;
	GetWorld()->LineTraceSingleByObjectType(Hit, PlayerViewLocation, LineTraceEnd, ObjectQueryParams, TraceParams);

	// Debug
	// DrawDebugLine(GetWorld(), PlayerViewLocation, LineTraceEnd, FColor::Green, false, 20.0f);
	if (IsValid(Hit.GetActor()))
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, *FString(Hit.GetActor()->GetName()));
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, *FString(Hit.GetComponent()->GetName()));
	}

	return Hit;
}

void UGrabberComponent::GetPlayerViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	PlayerController->GetPlayerViewPoint(OutLocation, OutRotation);
}

void UGrabberComponent::GrabThrow()
{
	if (GrabbedItem == nullptr)
		Grab();
	else
		Throw();
}

void UGrabberComponent::Grab()
{
	if (!IsValid(PhysicsHandle)) return;

	FHitResult HitResult = GetFirstPhysicsBodyInReach();
	UPrimitiveComponent* ComponentToGrab = HitResult.GetComponent();
	AActor* HitActor = HitResult.GetActor();

	if (!IsValid(HitActor))
		return;

	if (HitActor->ActorHasTag("CanBeCaptured"))
	{
		ShowWinWidget();
		return;
	}

	if (IsValid(ComponentToGrab))
	{
		if (ComponentToGrab->IsSimulatingPhysics())
		{
			GrabbedItem = HitActor;

			FVector GrabLocation = ComponentToGrab->GetCenterOfMass();
			FRotator GrabRotation = ComponentToGrab->GetComponentRotation();

			PhysicsHandle->GrabComponentAtLocationWithRotation(ComponentToGrab, NAME_None, GrabLocation, GrabRotation);
		}
	}
}

void UGrabberComponent::Throw()
{
	if (!PhysicsHandle) return;

	UPrimitiveComponent* GrabbedComponent = PhysicsHandle->GetGrabbedComponent();
	if (!GrabbedComponent)
	{
		PhysicsHandle->ReleaseComponent();
		return;
	}

	// Apply impulse to throw
	FVector PlayerViewLocation;
	FRotator PlayerViewRotation;
	GetPlayerViewPoint(PlayerViewLocation, PlayerViewRotation);

	const FVector ThrowDirection = PlayerViewRotation.Vector().GetSafeNormal();
	const FVector Impulse = ThrowDirection * ThrowStrength;
	
	// Add impulse at component's center
	PhysicsHandle->ReleaseComponent();
	GrabbedComponent->AddImpulse(Impulse, NAME_None, true);
	
	GrabbedItem = nullptr;
}

void UGrabberComponent::ShowWinWidget()
{
	if (!WinWidgetClass) return;

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController) return;

	UUserWidget* WinWidget = CreateWidget<UUserWidget>(PlayerController, WinWidgetClass);
	if (WinWidget)
	{
		GrabbedItem = nullptr;
		WinWidget->AddToViewport();
		PlayerController->SetShowMouseCursor(true);
		PlayerController->SetInputMode(FInputModeUIOnly());
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.0f);
	}
}
