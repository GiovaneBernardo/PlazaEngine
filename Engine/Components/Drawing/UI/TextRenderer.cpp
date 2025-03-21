#include "Engine/Core/PreCompiledHeaders.h"
#include "TextRenderer.h"

namespace Plaza::Drawing::UI {
	TextRenderer::TextRenderer() {
		// Init(Application::Get()->enginePath + "/Font/Roboto-Medium.ttf");
	}

	void TextRenderer::Init(std::string fontPath) {
		// FT_Library ft;
		// if (FT_Init_FreeType(&ft))
		//{
		//	std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		//	return;
		// }
		//
		//// load font as face
		// FT_Face face;
		// if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
		//	std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
		//	return;
		// }
		// else {
		//	// set size to load glyphs as
		//	FT_Set_Pixel_Sizes(face, 0, 48);
		//
		//	// disable byte-alignment restriction
		//	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		//
		//	// load first 128 characters of ASCII set
		//	for (unsigned char c = 0; c < 128; c++)
		//	{
		//		// Load character glyph
		//		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		//		{
		//			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
		//			continue;
		//		}
		//		// generate texture
		//		unsigned int texture;
		//		glGenTextures(1, &texture);
		//		glBindTexture(GL_TEXTURE_2D, texture);
		//		glTexImage2D(
		//			GL_TEXTURE_2D,
		//			0,
		//			GL_RED,
		//			face->glyph->bitmap.width,
		//			face->glyph->bitmap.rows,
		//			0,
		//			GL_RED,
		//			GL_UNSIGNED_BYTE,
		//			face->glyph->bitmap.buffer
		//		);
		//		// set texture options
		//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//		// now store character for later use
		//		Character character = {
		//			texture,
		//			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
		//			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
		//			static_cast<unsigned int>(face->glyph->advance.x)
		//		};
		//		mCharacters.insert(std::pair<char, Character>(c, character));
		//	}
		//	glBindTexture(GL_TEXTURE_2D, 0);
		// }
		//// destroy FreeType once we're finished
		// FT_Done_Face(face);
		// FT_Done_FreeType(ft);
		//
		// glGenVertexArrays(1, &mVAO);
		// glGenBuffers(1, &mVBO);
		// glBindVertexArray(mVAO);
		// glBindBuffer(GL_ARRAY_BUFFER, mVBO);
		// glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
		// glEnableVertexAttribArray(0);
		// glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		// glBindBuffer(GL_ARRAY_BUFFER, 0);
		// glBindVertexArray(0);
	}
	void TextRenderer::Render() {
		// float x = mPosX;
		// float y = mPosY;
		// glEnable(GL_CULL_FACE);
		// glEnable(GL_BLEND);
		// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// glm::mat4 projection = glm::ortho(0.0f, Application::Get()->appSizes->sceneSize.x, 0.0f,
		// static_cast<float>(Application::Get()->appSizes->sceneSize.y)); shader.use();
		// glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		//// activate corresponding render state
		// glUniform3f(glGetUniformLocation(shader.ID, "textColor"), mColor.x, mColor.y, mColor.z);
		// glActiveTexture(GL_TEXTURE0);
		// glBindVertexArray(mVAO);
		//
		//// iterate through all characters
		// std::string::const_iterator c;
		// for (c = mText.begin(); c != mText.end(); c++)
		//{
		//	Character ch = mCharacters[*c];
		//
		//	float xpos = x + ch.Bearing.x * mScale;
		//	float ypos = y - (ch.Size.y - ch.Bearing.y) * mScale;
		//
		//	float w = ch.Size.x * mScale;
		//	float h = ch.Size.y * mScale;
		//	// update VBO for each character
		//	float vertices[6][4] = {
		//		{ xpos,     ypos + h,   0.0f, 0.0f },
		//		{ xpos,     ypos,       0.0f, 1.0f },
		//		{ xpos + w, ypos,       1.0f, 1.0f },
		//
		//		{ xpos,     ypos + h,   0.0f, 0.0f },
		//		{ xpos + w, ypos,       1.0f, 1.0f },
		//		{ xpos + w, ypos + h,   1.0f, 0.0f }
		//	};
		//	// render glyph texture over quad
		//	glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		//	// update content of VBO memory
		//	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
		//	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not
		//glBufferData
		//
		//	glBindBuffer(GL_ARRAY_BUFFER, 0);
		//	// render quad
		//	glDrawArrays(GL_TRIANGLES, 0, 6);
		//	// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		//	x += (ch.Advance >> 6) * mScale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th
		//pixels by 64 to get amount of pixels))
		// }
		// glBindVertexArray(0);
		// glBindTexture(GL_TEXTURE_2D, 0);
	}
	void TextRenderer::Terminate() {
		// glDeleteBuffers(1, &mVAO);
		// glDeleteBuffers(1, &mVBO);
	}
} // namespace Plaza::Drawing::UI
