#include <PCH.h>

#include <glad/glad.h>

#include "OpenGLVertexBuffer.h"

namespace PetrolEngine {
	OpenGLVertexBuffer::OpenGLVertexBuffer(VertexLayout layout, const void* data, int64 size): VertexBuffer(layout) { LOG_FUNCTION();
		this->layout = layout;
		
		glGenBuffers(1, &ID);

		glBindBuffer(GL_ARRAY_BUFFER, ID);

		glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW); //GL_STATIC_DRAW
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(VertexLayout layout): VertexBuffer(layout) { LOG_FUNCTION();
		this->layout = layout;

		glGenBuffers(1, &ID);
	}

	void OpenGLVertexBuffer::setData(const void* data, int64 size) {
		LOG_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, ID);

		glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer() { LOG_FUNCTION();
		glDeleteBuffers(1, &ID);
	}
}
