// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include <NewShooter/NewShooterCharacter.h>
#include "../PlayerController/TPSPlayerController.h"

//子弹会在所有客户端生成
void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ANewShooterCharacter* OwnerCharacter = Cast<ANewShooterCharacter>(GetOwner());
	ANewShooterCharacter* OtherCharacter = Cast<ANewShooterCharacter>(OtherActor);
	if (OwnerCharacter && OtherCharacter)
	{
		
		ATPSPlayerController* OwnerController = Cast<ATPSPlayerController>(OwnerCharacter->Controller);
		if (OwnerController)
		{
			
			OwnerController->ServerHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit, Damage);
			OwnerCharacter->HitEnemy();
		}
	}

	//销毁
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}


