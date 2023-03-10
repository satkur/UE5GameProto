// Fill out your copyright notice in the Description page of Project Settings.


#include "PawnWithCamera.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet\KismetSystemLibrary.h"

// Sets default values
APawnWithCamera::APawnWithCamera() {
    // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // create components
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    staticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
    springArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
    cameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));

    // set mesh
    ConstructorHelpers::FObjectFinder<UStaticMesh> capsuleMesh(TEXT("/Game/StarterContent/Shapes/Shape_NarrowCapsule.Shape_NarrowCapsule"));
    if (capsuleMesh.Succeeded()) {
        staticMeshComp->SetStaticMesh(capsuleMesh.Object);
    }

    // attach components
    staticMeshComp->SetupAttachment(RootComponent);
    springArmComp->SetupAttachment(staticMeshComp);
    cameraComp->SetupAttachment(springArmComp, USpringArmComponent::SocketName);

    // assign SpringArm variables
    springArmComp->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 50.f), FRotator(-60.0f, 0.0f, 0.0f));
    springArmComp->TargetArmLength = 400.0f;
    springArmComp->bEnableCameraLag = true; // camera smooth
    springArmComp->CameraLagSpeed = 3.0f;

    // take control of the default Player
    AutoPossessPlayer = EAutoReceiveInput::Player0;
}

// Called when the game starts or when spawned
void APawnWithCamera::BeginPlay() {
    Super::BeginPlay();

}

// Called every frame
void APawnWithCamera::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

    // set camera FOV and arm length by zooming in/out
    {
        if (bZoomingIn) {
            zoomFactor += DeltaTime / 0.5f;
        } else {
            zoomFactor -= DeltaTime / 0.25f;
        }
        zoomFactor = FMath::Clamp<float>(zoomFactor, 0.0f, 1.0f);

        cameraComp->FieldOfView = FMath::Lerp<float>(90.0f, 60.0f, zoomFactor);
        springArmComp->TargetArmLength = FMath::Lerp<float>(400.0f, 300.0f, zoomFactor);
    }

    // set actor yaw by cameraInput
    {
        FRotator actorRotation = GetActorRotation();
        actorRotation.Yaw += cameraInput.X;
        SetActorRotation(actorRotation);
    }

    // set camera arm pitch by cameraInput
    {
        FRotator armRotation = springArmComp->GetComponentRotation();
        armRotation.Pitch = FMath::Clamp(armRotation.Pitch + cameraInput.Y, -80.0f, -15.0f);
        springArmComp->SetWorldRotation(armRotation);
    }

    // actor movement by input
    {
        if (!movementInput.IsZero()) {
            FVector2D inputNormal = movementInput.GetSafeNormal();
            FVector actorLocation = GetActorLocation();
            actorLocation += GetActorForwardVector() * inputNormal.X * DeltaTime * 300;
            actorLocation += GetActorRightVector() * inputNormal.Y * DeltaTime * 300;
            SetActorLocation(actorLocation);
        }
    }
}

// Called to bind functionality to input
void APawnWithCamera::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // hook zoomIn
    InputComponent->BindAction("ZoomIn", IE_Pressed, this, &APawnWithCamera::zoomIn);
    InputComponent->BindAction("ZoomIn", IE_Released, this, &APawnWithCamera::zoomOut);

    // hook input movement
    InputComponent->BindAxis("MoveForward", this, &APawnWithCamera::moveForward);
    InputComponent->BindAxis("MoveRight", this, &APawnWithCamera::moveRight);
    InputComponent->BindAxis("CameraPitch", this, &APawnWithCamera::pitchCamera);
    InputComponent->BindAxis("CameraYaw", this, &APawnWithCamera::yawCamera);
}

void APawnWithCamera::moveForward(float axisValue) {
    movementInput.X = FMath::Clamp<float>(axisValue, -1.0f, 1.0f);
}

void APawnWithCamera::moveRight(float axisValue) {
    movementInput.Y = FMath::Clamp<float>(axisValue, -1.0f, 1.0f);
}

void APawnWithCamera::pitchCamera(float axisValue) {
    cameraInput.Y = axisValue;
}

void APawnWithCamera::yawCamera(float axisValue) {
    cameraInput.X = axisValue;
}

void APawnWithCamera::zoomIn() {
    bZoomingIn = true;
}

void APawnWithCamera::zoomOut() {
    bZoomingIn = false;
}
