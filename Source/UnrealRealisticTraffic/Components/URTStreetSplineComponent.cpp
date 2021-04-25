// Fill out your copyright notice in the Description page of Project Settings.


#include "URTStreetSplineComponent.h"

void UURTStreetSplineComponent::AdaptLeftTurnForm()
{
    ClearSplinePoints(true);

    FSplinePoint FirstPoint;
    FirstPoint.InputKey = 0;
    FirstPoint.Type = ESplinePointType::Type::CurveCustomTangent;
    FirstPoint.Position = FVector::ZeroVector;
    FirstPoint.ArriveTangent = FVector(-34.f, -416.f, 0.f);
    FirstPoint.LeaveTangent = FVector(-34.f, -416.f, 0.f);
    FirstPoint.Rotation = FRotator::ZeroRotator;
    FirstPoint.Scale = FVector(1.f, 1.f, 1.f);

    AddPoint(FirstPoint, true);

    FSplinePoint SecondPoint;
    SecondPoint.InputKey = 1;
    SecondPoint.Type = ESplinePointType::Type::CurveCustomTangent;
    SecondPoint.Position = FVector(-197.f, -432.f, 0.f);
    SecondPoint.ArriveTangent = FVector(-794.f, 90.f, 0.f);
    SecondPoint.LeaveTangent = FVector(-794.f, 90.f, 0.f);
    SecondPoint.Rotation = FRotator::ZeroRotator;
    SecondPoint.Scale = FVector(1.f, 1.f, 1.f);

    AddPoint(SecondPoint, true);

    FSplinePoint ThirdPoint;
    ThirdPoint.InputKey = 2;
    ThirdPoint.Type = ESplinePointType::Type::CurveCustomTangent;
    ThirdPoint.Position = FVector(-560.f, -432.f, 0.f);
    ThirdPoint.ArriveTangent = FVector(28.f, 22.f, 0.f);
    ThirdPoint.LeaveTangent = FVector(28.f, 22.f, 0.f);
    ThirdPoint.Rotation = FRotator::ZeroRotator;
    ThirdPoint.Scale = FVector(1.f, 1.f, 1.f);

    AddPoint(ThirdPoint, true);
}

void UURTStreetSplineComponent::AdaptForwardForm()
{

}

void UURTStreetSplineComponent::AdaptRightTurnForm()
{
    ClearSplinePoints(true);

    FSplinePoint FirstPoint;
    FirstPoint.InputKey = 0;
    FirstPoint.Type = ESplinePointType::Type::CurveCustomTangent;
    FirstPoint.Position = FVector::ZeroVector;
    FirstPoint.ArriveTangent = FVector(-34.f, -416.f, 0.f);
    FirstPoint.LeaveTangent = FVector(-34.f, -416.f, 0.f);
    FirstPoint.Rotation = FRotator::ZeroRotator;
    FirstPoint.Scale = FVector(1.f, 1.f, 1.f);

    AddPoint(FirstPoint, true);

    FSplinePoint SecondPoint;
    SecondPoint.InputKey = 1;
    SecondPoint.Type = ESplinePointType::Type::CurveCustomTangent;
    SecondPoint.Position = FVector(144.f, -432.f, 0.f);
    SecondPoint.ArriveTangent = FVector(794.f, 90.f, 0.f);
    SecondPoint.LeaveTangent = FVector(794.f, 90.f, 0.f);
    SecondPoint.Rotation = FRotator::ZeroRotator;
    SecondPoint.Scale = FVector(1.f, 1.f, 1.f);

    AddPoint(SecondPoint, true);

    FSplinePoint ThirdPoint;
    ThirdPoint.InputKey = 2;
    ThirdPoint.Type = ESplinePointType::Type::CurveCustomTangent;
    ThirdPoint.Position = FVector(460.f, -432.f, 0.f);
    ThirdPoint.ArriveTangent = FVector(28.f, 22.f, 0.f);
    ThirdPoint.LeaveTangent = FVector(28.f, 22.f, 0.f);
    ThirdPoint.Rotation = FRotator::ZeroRotator;
    ThirdPoint.Scale = FVector(1.f, 1.f, 1.f);

    AddPoint(ThirdPoint, true);
}
