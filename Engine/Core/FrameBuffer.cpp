#include "Engine/Core/PreCompiledHeaders.h"
#include "FrameBuffer.h"
namespace Engine {
	void FrameBuffer::Init() {
		glGenFramebuffers(1, &this->buffer);
		glBindFramebuffer(this->target, this->buffer);
	}

	void FrameBuffer::InitRenderBufferObject(GLenum internalFormat, int width, int height) {
		glGenRenderbuffers(1, &this->renderBufferObject);
		glBindRenderbuffer(GL_RENDERBUFFER, this->renderBufferObject);
		glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
		glViewport(0, 0, width, height);
		glFramebufferRenderbuffer(this->target, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->renderBufferObject);
	}

	void FrameBuffer::InitColorAttachment(GLenum textureTarget, GLenum internalFormat, int width, int height, GLenum format, GLenum type, GLint param) {
		glGenTextures(1, &this->colorBuffer);
		glBindTexture(textureTarget, this->colorBuffer);
		glTexImage2D(textureTarget, 0, internalFormat, width, height, 0, format, type, NULL);
		glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, param);
		glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, param);
		glFramebufferTexture2D(textureTarget, GL_COLOR_ATTACHMENT0, textureTarget, this->colorBuffer, 0);
		glBindTexture(this->target, 0);
	}

	void FrameBuffer::InitDepthAttachment(GLint level, GLenum internalFormat, int width, int height, GLenum format, GLenum type) {

	}

	void FrameBuffer::DrawAttachments(GLenum attachments[]) {
		glDrawBuffers(sizeof(attachments) / sizeof(attachments[0]), attachments);
		std::cout << sizeof(attachments) / sizeof(attachments[0]) << std::endl;
	}

	void FrameBuffer::UpdateSize()
	{

	}

	void FrameBuffer::Bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, this->buffer);
	}

	void FrameBuffer::Unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FrameBuffer::Terminate()
	{
		glDeleteBuffers(1, &this->buffer);
	}
}