// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Grid.h"
#include "ControllerPawn.generated.h"

UCLASS()
class MICEMEN_API AControllerPawn : public APawn
{
	GENERATED_BODY()

	/** Dummy root component */
	UPROPERTY(Category = Board, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* DummyRoot;

public:
	// Sets default values for this pawn's properties
	AControllerPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Board Settings")
	AGrid* GameBoard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	FVector LocationOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	FRotator RotationOffset;

	UFUNCTION(BlueprintCallable)
	void MoveUp();

	UFUNCTION(BlueprintCallable)
	void MoveDown();

	UFUNCTION(BlueprintCallable)
	void MoveRight();

	UFUNCTION(BlueprintCallable)
	void MoveLeft();

	UPROPERTY(VisibleAnywhere, Category = "Board Settings")
	int32 SelectedColumn;
	int32 PreviousColumn;

	float InputDelay;
	float ErrorMargin;

	int32 PreviousMovedColumn;
	TArray<int32> TeamColumns;

	int32 CurrentTeam;
	int32 ColumnIterator;

	bool bReady = false;

	void SwapActiveTeam();
	void UpdateColumns();
	void RemoveInvalidColumn();
	void LevelReset();

	bool bFirstUpdate = true;

	TArray<int32> BluePreviousMoves;
	TArray<int32> RedPreviousMoves;

	void MoveSelectedColumn(bool bUpward);

	int32 CountEqualMoves(TArray<int32> TeamMovesArray, int32 Move);

	UFUNCTION(BlueprintCallable)
	void FinishMatch();

	UFUNCTION(BlueprintCallable)
	bool IsFinished();

	bool bFinished = false;

	int32 TurnsBeforeDraw = 8;
	bool bDrawContdownBegan = false;

	bool bDraw = false;

	UFUNCTION(BlueprintCallable)
	bool DidMatchDraw();
};
