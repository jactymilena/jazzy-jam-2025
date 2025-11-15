// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MapGeneration/MapGenerator.h"
#include "NinetyNinePinkBallsGameMode.generated.h"

/**
 *  Simple GameMode for a first person game
 */
UCLASS(abstract)
class ANinetyNinePinkBallsGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANinetyNinePinkBallsGameMode();
	
protected:
	virtual void BeginPlay() override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* newPlayer) override;
	
private:
	void RegisterToMapReadyEvent();
	void OnMapReady();

	TArray<TWeakObjectPtr<APlayerController>> _waitingPlayerControllers;
	
	bool _isMapReady = false;
	FDelegateHandle _mapReadyHandle;
	
	TWeakObjectPtr<AMapGenerator> _mapGenerator;
};



