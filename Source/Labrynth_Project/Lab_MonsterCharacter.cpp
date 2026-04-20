#include "Lab_MonsterCharacter.h"
#include "Lab_GameMode.h"
#include "Lab_PlayerController.h"
#include "Lab_PlayerState.h"
#include "Lab_SurvivorCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

ALab_MonsterCharacter::ALab_MonsterCharacter()
{
	bReplicates = true;
	SetReplicatingMovement(true);

	bUseControllerRotationYaw   = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll  = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	GetMesh()->SetOwnerNoSee(true);

	TagSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TagSphere"));
	TagSphere->SetupAttachment(RootComponent);
	TagSphere->SetSphereRadius(100.f);
	TagSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void ALab_MonsterCharacter::BeginPlay()
{
	Super::BeginPlay();

	TagSphere->OnComponentBeginOverlap.AddDynamic(this, &ALab_MonsterCharacter::OnTagSphereOverlap);
}

void ALab_MonsterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (ALab_PlayerController* PC = Cast<ALab_PlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Sub =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
				Sub->AddMappingContext(DefaultMappingContext, 0);
			if (MouseLookMappingContext)
				Sub->AddMappingContext(MouseLookMappingContext, 1);
		}
		PC->EnableGameInputMode();
	}

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALab_MonsterCharacter::Move);
		if (LookAction)
			EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALab_MonsterCharacter::Look);
	}
}

void ALab_MonsterCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (!Controller) return;

	const FRotator Yaw(0.f, Controller->GetControlRotation().Yaw, 0.f);
	AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::X), Axis.Y);
	AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y), Axis.X);
}

void ALab_MonsterCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ALab_MonsterCharacter::OnTagSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                                bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	ALab_SurvivorCharacter* Survivor = Cast<ALab_SurvivorCharacter>(OtherActor);
	if (!Survivor) return;

	APlayerController* SurvivorPC = Cast<APlayerController>(Survivor->GetController());
	if (!SurvivorPC) return;

	// Get name before NotifySurvivorCaught destroys the survivor pawn
	ALab_PlayerState* SurvivorPS = Survivor->GetPlayerState<ALab_PlayerState>();
	const FString Name = SurvivorPS ? SurvivorPS->GetPlayerName() : TEXT("A Survivor");

	if (ALab_GameMode* GM = GetWorld()->GetAuthGameMode<ALab_GameMode>())
	{
		GM->NotifySurvivorCaught(SurvivorPC);
	}

	Multicast_OnSurvivorTagged(Name);
}

void ALab_MonsterCharacter::Multicast_OnSurvivorTagged_Implementation(const FString& SurvivorName)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			FString::Printf(TEXT("Monster tagged %s!"), *SurvivorName));
	}
}
