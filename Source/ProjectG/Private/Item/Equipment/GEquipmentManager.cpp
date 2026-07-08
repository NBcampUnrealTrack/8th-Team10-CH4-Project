// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Equipment/GEquipmentManager.h"

// Sets default values for this component's properties
UGEquipmentManager::UGEquipmentManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGEquipmentManager::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UGEquipmentManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

