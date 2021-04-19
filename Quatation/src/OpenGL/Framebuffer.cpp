#include "Framebuffer.h"

#include <glad/glad.h>

namespace choice
{
	static uint32_t sMaxFramebufferSize = 8192;
	Framebuffer::Framebuffer(uint32_t w, uint32_t h)
		:mWidth(w), mHeight(h) {}

	Framebuffer::~Framebuffer()
	{
		if (mRendererId) { Destroy(); }
	}

	void Framebuffer::Bind()const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mRendererId);
		glViewport(0, 0, mWidth, mHeight);
	}

	void Framebuffer::BindRead() const
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, mRendererId);
	}

	void Framebuffer::BindDraw() const
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mRendererId);
	}

	void Framebuffer::UnBind()const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	const uint32_t& Framebuffer::GetWidth() const
	{
		return mWidth;
	}

	const uint32_t& Framebuffer::GetHeight() const
	{
		return mHeight;
	}

	void Framebuffer::Visible(uint32_t w, uint32_t h)
	{
		mWidth = w; mHeight = h;
		Invalidate();
	}

	void Framebuffer::Destroy()
	{
		if (mRendererId) { glDeleteFramebuffers(1, &mRendererId); }
	}
}