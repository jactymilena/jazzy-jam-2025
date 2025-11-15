	// Copyright Epic Games, Inc. All Rights Reserved.

#include "NinetyNinePinkBallsGameMode.h"

#include "Kismet/GameplayStatics.h"
#include "MapGeneration/MapGenerator.h"

ANinetyNinePinkBallsGameMode::ANinetyNinePinkBallsGameMode()
{
	// stub
}

void ANinetyNinePinkBallsGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	RegisterToMapReadyEvent();
}

void ANinetyNinePinkBallsGameMode::HandleStartingNewPlayer_Implementation(APlayerController* newPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(newPlayer);
	if (!_isMapReady)
	{
		RegisterToMapReadyEvent();
		_waitingPlayerControllers.Add(newPlayer);
	}
}

void ANinetyNinePinkBallsGameMode::RegisterToMapReadyEvent()
{
	if (_mapReadyHandle.IsValid() || _isMapReady) { return; }
	
	_mapGenerator = Cast<AMapGenerator>(UGameplayStatics::GetActorOfClass(GetWorld(), AMapGenerator::StaticClass()));
	if (_mapGenerator.IsValid())
	{
		_mapReadyHandle = _mapGenerator->OnMapReady.AddUObject(this, &ThisClass::OnMapReady);	
	}
}

void ANinetyNinePinkBallsGameMode::OnMapReady()
{
	_isMapReady = true;
	for (TWeakObjectPtr<APlayerController> playerController : _waitingPlayerControllers)
	{
		if (playerController.IsValid())
		{
			if (IsValid(playerController->GetPawn()) && _mapGenerator.IsValid())
			{
				APawn* playerPawn = playerController->GetPawn();
				FVector position =_mapGenerator->GetPlayerStartPosition();
				playerPawn->SetActorLocation(position, false, nullptr, ETeleportType::TeleportPhysics);
			}
		}
	}
}
