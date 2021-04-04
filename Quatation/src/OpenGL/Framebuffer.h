#pragma once
#include "cpch.h"

namespace choice
{
	class Framebuffer
	{
	public:
		Framebuffer(uint32_t w, uint32_t h);
		~Framebuffer();

		void Bind()const;
		void UnBind()const;

		void Visible(uint32_t w, uint32_t h);
	protected:
		void Destroy();
		virtual void Invalidate() = 0;
		uint32_t mRendererId;
		uint32_t mWidth, mHeight;
	};
}