#pragma once

#include "Core/Renderer/GraphicsContext.h"

namespace PetrolEngine {
	class OpenGLContext : public GraphicsContext {
	public:
		int init(void* loaderProc) override;
	};
}