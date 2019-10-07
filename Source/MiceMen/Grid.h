// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grid.generated.h"

UCLASS()
class MICEMEN_API AGrid : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrid();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	TMap<FIntPoint, AActor*> BlockMap;
	
	void GridInitialization();
	void Populate();
	bool SettleBoard();
	void PaintColumn(int32 column);
	void CheckGoal(int32 GoalPosition);
	void AddToScore(bool bIsBlue);

	UFUNCTION(BlueprintCallable, Category = "Board Settings")
	void AddBlock(FIntPoint Coordinates, int32 type);

	UFUNCTION(BlueprintCallable, Category = "Board Functions")
	void MoveColumn(int32 HorizontalCoordinate, bool Upward);

//private::

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Board Settings")
	float IterationOffset;

	UPROPERTY(EditAnywhere, Category = "Board Settings")
	UStaticMesh* CheeseMesh;

	UPROPERTY(EditAnywhere, Category = "Board Settings")
	UStaticMesh* MiceMesh;

	TArray<int32> TeamColumns(int32 Team);
	TArray<AActor*> BlueTeam;
	TArray<AActor*> RedTeam;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Board State")
	int32 BlueScore = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Board State")
	int32 RedScore = 0;

	bool bCanSettle = true;
};
