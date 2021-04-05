#pragma once
#include "cpch.h"

#include "Pipeline.h"
#include "OpenGL/Framebuffer.h"
#include "OpenGL/Shader.h"

#include "Project/Scene/Scene.h"

#include <glm/glm.hpp>

namespace choice
{
	class DeferredGeometryCapture :public Framebuffer
	{
	public:
		DeferredGeometryCapture(uint32_t w, uint32_t h);
		~DeferredGeometryCapture();

		void BindGBuffer(glm::uvec4 slots);
	private:
		void Invalidate()override;
		uint32_t mAlbedoSId, mPositionId, mNormalId, mDepthId;
	};

	class DeferredPipeline :public Pipeline
	{
	public:
		void Init(uint32_t w, uint32_t h)override;
		void Visible(uint32_t w, uint32_t h)override;
		void Update(Scene* scene, Camera* camera)override;
		void Shutdown()override;
	private:
		std::pair<DeferredGeometryCapture*, Shader*> mGeometryPass;
		Shader* mLightingPass;
	};
}