#pragma once

#include <Core/Aliases.h>

#include <Core/Renderer/Framebuffer.h>

namespace PetrolEngine{

    class OpenGLFramebuffer : public Framebuffer{
    public:
        OpenGLFramebuffer(const FramebufferSpecification& spec);

        void addAttachment(Texture*& texture) override;

        ~OpenGLFramebuffer() override;

    };

}