#pragma once

#include "Core/Renderer/Texture.h"
#include <glad/glad.h>

namespace PetrolEngine {

	class OpenGLTexture : public Texture {
	public:
		OpenGLTexture(
			int width,
			int height,
			TextureFormat format = TextureFormat::NONE,
            TextureType type = TextureType::Texture2D
		);

        OpenGLTexture(const Image& image);
		
		~OpenGLTexture() override;

		void updateTextureImage(const void* data, int index) override;
	private:
		const UnorderedMap<TextureFormat, Pair<GLuint, GLuint>> textureFormatLookupTable{
			{TextureFormat::RGBA16, {GL_RGBA, GL_RGBA16}},
			{TextureFormat::RGBA8 , {GL_RGBA, GL_RGBA8 }},
			{TextureFormat::RGB16 , {GL_RGB , GL_RGB16 }},
      {TextureFormat::RGB8  , {GL_RGB , GL_RGB8  }},
			{TextureFormat::RED   , {GL_RED , GL_RED   }},
			{TextureFormat::DEPTH24STENCIL8, {GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL}}
		};

        const UnorderedMap<TextureType, GLuint> textureTypeLookupTable{
                {TextureType::TextureCube, GL_TEXTURE_CUBE_MAP},
                {TextureType::Texture2D  , GL_TEXTURE_2D      },
                {TextureType::Texture3D  , GL_TEXTURE_3D      }
        };
	};
}
