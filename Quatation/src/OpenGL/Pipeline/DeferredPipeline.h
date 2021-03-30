#pragma once
#include "cpch.h"

#include "Pipeline.h"
#include "OpenGL/Framebuffer.h"
#include "OpenGL/Shader.h"

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

	struct DeferredPipeline :public Pipeline
	{
		void Init(uint32_t w, uint32_t h)override;
		void Visible(uint32_t w, uint32_t h)override;
		void Shutdown()override;

		std::pair<DeferredGeometryCapture*, Shader*> GeometryPass;
		Shader* LightingPass;
		PipelineTypes Type = PipelineTypes::DEFERRED;
	};
}