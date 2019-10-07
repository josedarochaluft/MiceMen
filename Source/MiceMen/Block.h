// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Grid.h"
#include "Materials/Material.h"
#include "UObject/ObjectMacros.h"
#include "GameFramework/Actor.h"
#include "Block.generated.h"

UENUM()
enum class EType
{
	Block,
	Blue,
	Red
};

UCLASS()
class MICEMEN_API ABlock : public AActor
{
	GENERATED_BODY()

	// Dummy Root Component
	UPROPERTY(Category = Block, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* DummyRoot;

	UPROPERTY(Category = Block, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* BlockMesh;
	
public:	
	// Sets default values for this actor's properties
	ABlock();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Board Info")
	FIntPoint Coordinates;

	UPROPERTY()
	UMaterial* BaseMaterial;

	UPROPERTY()
	UMaterialInstance* BlueMaterial;

	UPROPERTY()
	UMaterialInstance* RedMaterial;

	UPROPERTY()
	UMaterialInstance* HighlightMaterial;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* VisualMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MoveSpeed = 5.0f;

	void SetCoordinates(FIntPoint point);

	void SetType(EType type);

	void SetMesh(UStaticMesh* mesh);

	void SetMaterial(UMaterialInstance* material);
	void SetMaterial(UMaterial* material);

	void MoveTo(FVector TargetLocation, int32 MovementDistanceInBlocks);

	void UpdateNeighbours();

	void HighlightToggle();

	UPROPERTY(VisibleAnywhere)
	AActor* Below;

	UPROPERTY(VisibleAnywhere)
	AActor* Ahead;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	AGrid* GameBoard;

	EType ActorType;

	bool IsFalling();
	bool CanWalk();

	UFUNCTION(BlueprintCallable)
	int32 GetTypeInInt();

private:

	bool bPainted = false;
	bool bMoving = false;
	FVector MovementTargetLocation;
	FVector MovementStartLocation;
	float LengthMoved = 0.0f;
	float MovementIteration;
};
