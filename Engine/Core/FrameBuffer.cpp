#include "Engine/Core/PreCompiledHeaders.h"
#include "FrameBuffer.h"
namespace Plaza {
	void FrameBuffer::Init() {
		glGenFramebuffers(1, &this->buffer);
		glBindFramebuffer(this->target, this->buffer);
	}

	void FrameBuffer::InitRenderBufferObject(GLenum internalFormat, int width, int height) {
		glGenRenderbuffers(1, &this->renderBufferObject);
		glBindRenderbuffer(GL_RENDERBUFFER, this->renderBufferObject);
		glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
		glViewport(0, 0, width, height);
		glFramebufferRenderbuffer(this->target, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->renderBufferObject);
	}

	/*
			blurringBuffer->InitColorAttachment(GL_TEXTURE_2D, GL_RGB32F, width, height, GL_RGB, GL_FLOAT, GL_LINEAR);
		blurringBuffer->InitRenderBufferObject(GL_DEPTH24_STENCIL8, width, height);
	*/

	void FrameBuffer::InitColorAttachment(GLenum textureTarget, GLenum internalFormat, int width, int height,
										  GLenum format, GLenum type, GLint param) {
		glGenTextures(1, &this->colorBuffer);
		glBindTexture(textureTarget, this->colorBuffer);
		glTexImage2D(textureTarget, 0, internalFormat, width, height, 0, format, type, NULL);
		glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, param);
		glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, param);
		glFramebufferTexture2D(this->target, GL_COLOR_ATTACHMENT0, textureTarget, this->colorBuffer, 0);
		glBindTexture(this->target, 0);
	}

	void FrameBuffer::InitDepthAttachment(GLint level, GLenum internalFormat, int width, int height, GLenum format,
										  GLenum type) {}

	void FrameBuffer::DrawAttachments(GLenum attachments[], int width, int height) {
		glDrawBuffers(sizeof(attachments) / sizeof(attachments[0]), attachments);
		std::cout << sizeof(attachments) / sizeof(attachments[0]) << std::endl;
		glViewport(0, 0, width, height);
	}

	void FrameBuffer::UpdateSize() {}

	void FrameBuffer::Bind() { glBindFramebuffer(GL_FRAMEBUFFER, this->buffer); }

	void FrameBuffer::Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

	void FrameBuffer::Terminate() { glDeleteBuffers(1, &this->buffer); }
} // namespace Plaza
