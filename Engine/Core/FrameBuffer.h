#pragma once
namespace Plaza {
	class FrameBuffer {
	  public:
		unsigned int buffer;
		unsigned int colorBuffer;
		unsigned int depthBuffer;
		unsigned int renderBufferObject;
		GLenum target;

		FrameBuffer(const GLenum target) : target(target) { Init(); };

		void Init();
		void Bind();
		void Unbind();

		void InitRenderBufferObject(GLenum internalFormat, int width, int height);
		/// <summary>
		/// Example: (GL_TEXTURE_2D, GL_RGBA32F, x, y, GL_RGBA, GL_FLOAT, GL_LINEAR);
		/// </summary>
		/// <param name="textureTarget"></param>
		/// <param name="internalFormat"></param>
		/// <param name="width"></param>
		/// <param name="height"></param>
		/// <param name="format"></param>
		/// <param name="type"></param>
		/// <param name="param"></param>
		void InitColorAttachment(GLenum textureTarget, GLenum internalFormat, int width, int height, GLenum format,
								 GLenum type, GLint param);
		void InitDepthAttachment(GLint level, GLenum internalFormat, int width, int height, GLenum format, GLenum type);

		void DrawAttachments(GLenum attachments[], int width, int height);
		void UpdateSize();
		void Terminate();
	};
} // namespace Plaza
