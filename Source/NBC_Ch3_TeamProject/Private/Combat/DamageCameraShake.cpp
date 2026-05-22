// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/DamageCameraShake.h"

UDamageCameraShake::UDamageCameraShake()
{
	OscillationDuration = 0.25f;
	OscillationBlendInTime = 0.05f;
	OscillationBlendOutTime = 0.1f;

	// 상하 흔들림 (피격 임팩트의 주축)
	RotOscillation.Pitch.Amplitude = 2.5f;
	RotOscillation.Pitch.Frequency = 25.0f;
	RotOscillation.Pitch.InitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;

	// 좌우 흔들림
	RotOscillation.Yaw.Amplitude = 1.5f;
	RotOscillation.Yaw.Frequency = 20.0f;
	RotOscillation.Yaw.InitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;

	// 미세 위치 흔들림
	LocOscillation.Y.Amplitude = 1.0f;
	LocOscillation.Y.Frequency = 15.0f;
	LocOscillation.Y.InitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;
}
