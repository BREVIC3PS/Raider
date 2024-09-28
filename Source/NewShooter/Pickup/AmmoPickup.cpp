// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
AAmmoPickup::AAmmoPickup()
{
	RefreshDelay = 30;
}

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bActivated)return;
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	SphereOverlap(OtherActor);//Lua
}

void AAmmoPickup::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}


void AAmmoPickup::MulticastAddAmmo_Implementation(ANewShooterCharacter* CharacterToAttach)
{
}
