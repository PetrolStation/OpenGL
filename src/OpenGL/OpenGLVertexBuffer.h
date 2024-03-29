#pragma once

#include "Core/Renderer/VertexBuffer.h"

namespace PetrolEngine {
	class OpenGLVertexBuffer : public VertexBuffer {
	public:
		OpenGLVertexBuffer(VertexLayout layout);
		OpenGLVertexBuffer(VertexLayout layout, const void* data, int64 size);

		virtual void setData(const void* data, int64 size) override;

		~OpenGLVertexBuffer() override;

		const VertexLayout& getVertexLayout() { return layout; }
		
	private:
		VertexLayout layout;
	};
}
