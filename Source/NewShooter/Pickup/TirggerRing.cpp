// Fill out your copyright notice in the Description page of Project Settings.



#include "TirggerRing.h"

ATirggerRing::ATirggerRing()
{
    if (HasAuthority())
    {
        // Whenever an actor overlaps with this trigger box, call the OnOverlapBegin function.
        OnActorBeginOverlap.AddDynamic(this, &ATirggerRing::OnOverlapBegin);

        // Whenever an actor stops overlapping with this trigger box, call the OnOverlapEnd function.
        OnActorEndOverlap.AddDynamic(this, &ATirggerRing::OnOverlapEnd);
    }
}

void ATirggerRing::OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor)
{
    if (OtherActor && (OtherActor != this))
    {
    }
}

void ATirggerRing::OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor)
{
    if (OtherActor && (OtherActor != this))
    {
        
    }
}