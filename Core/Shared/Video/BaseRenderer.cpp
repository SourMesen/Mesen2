#include "pch.h"
#include <cmath>
#include "Shared/Video/BaseRenderer.h"

BaseRenderer::BaseRenderer(Emulator* emu)
{
	_emu = emu;
}

BaseRenderer::~BaseRenderer()
{
}
