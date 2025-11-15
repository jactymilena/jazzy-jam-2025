#include "Ball.h"
#include "GameFramework/Character.h"

ABall::ABall()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create the collision component
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SphereCollision->InitSphereRadius(50.f);
	SphereCollision->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	SphereCollision->SetSimulatePhysics(true);
	SphereCollision->SetNotifyRigidBodyCollision(true);
	
	RootComponent = SphereCollision;

	// Create the mesh
	SphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereMesh"));
	SphereMesh->SetupAttachment(RootComponent);
	SphereMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMeshAsset.Succeeded())
	{
		SphereMesh->SetStaticMesh(SphereMeshAsset.Object);
	}
}

void ABall::BeginPlay()
{
	Super::BeginPlay();

	SphereCollision->OnComponentHit.AddDynamic(this, &ABall::OnBallHit);
}

void ABall::OnBallHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || !OtherComp) return;
	
	// Only react if the thing hitting the ball is the player character
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character) return; // ignore everything else
	
	// Get the base direction AWAY from impact surface
	FVector BaseDir = Hit.ImpactNormal;

	// Remove vertical component to get horizontal direction
	FVector HorizontalDir = BaseDir;
	HorizontalDir.Z = 0.f;
	HorizontalDir.Normalize();

	// Rotate the horizontal direction upward by EXACT 25 degrees
	FRotator AngleUp(BounceAngleDegree, 0.f, 0.f);
	FVector BounceDir = AngleUp.RotateVector(HorizontalDir);

	// Apply the impulse
	FVector Impulse = BounceDir * BounceForce;
	SphereCollision->AddImpulse(Impulse, NAME_None, true);
}
