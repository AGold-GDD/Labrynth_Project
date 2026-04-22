#include "Lab_MonsterCharacter.h"
#include "Lab_GameMode.h"
#include "Lab_PlayerController.h"
#include "Lab_PlayerState.h"
#include "Lab_SurvivorCharacter.h"
#include "Lab_NameplateWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
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

	NameplateWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameplateWidget"));
	NameplateWidget->SetupAttachment(RootComponent);
	NameplateWidget->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	NameplateWidget->SetWidgetClass(ULab_NameplateWidget::StaticClass());
	NameplateWidget->SetWidgetSpace(EWidgetSpace::Screen);
	NameplateWidget->SetDrawSize(FVector2D(200.f, 50.f));
}

void ALab_MonsterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (ULab_NameplateWidget* NW = Cast<ULab_NameplateWidget>(NameplateWidget->GetUserWidgetObject()))
		NW->SetOwningCharacter(this);

	if (IsLocallyControlled())
		NameplateWidget->SetVisibility(false);
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
		if (TagAction)
			EIC->BindAction(TagAction, ETriggerEvent::Started, this, &ALab_MonsterCharacter::PerformTag);
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

void ALab_MonsterCharacter::PerformTag()
{
	if (!FirstPersonCamera) return;

	const FVector Start = FirstPersonCamera->GetComponentLocation();
	const FVector End   = Start + FirstPersonCamera->GetForwardVector() * TagRange;

	// Dev visualisation — visible only on the monster's screen
	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.5f, 0, 2.f);

	if (HasAuthority())
		Server_TryTag_Implementation();
	else
		Server_TryTag();
}

void ALab_MonsterCharacter::Server_TryTag_Implementation()
{
	if (!FirstPersonCamera) return;

	const FVector Start = FirstPersonCamera->GetComponentLocation();
	const FVector End   = Start + FirstPersonCamera->GetForwardVector() * TagRange;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		return;

	ALab_SurvivorCharacter* Survivor = Cast<ALab_SurvivorCharacter>(Hit.GetActor());
	if (!Survivor) return;

	APlayerController* SurvivorPC = Cast<APlayerController>(Survivor->GetController());
	if (!SurvivorPC) return;

	ALab_PlayerState* SurvivorPS = Survivor->GetPlayerState<ALab_PlayerState>();
	const FString Name = (SurvivorPS && !SurvivorPS->DisplayName.IsEmpty())
		? SurvivorPS->DisplayName
		: TEXT("A Survivor");

	if (ALab_GameMode* GM = GetWorld()->GetAuthGameMode<ALab_GameMode>())
		GM->NotifySurvivorCaught(SurvivorPC);

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
