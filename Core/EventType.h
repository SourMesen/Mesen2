#pragma once

enum class EventType
{
	Nmi = 0,
	Irq = 1,
	StartFrame = 2,
	EndFrame = 3
};