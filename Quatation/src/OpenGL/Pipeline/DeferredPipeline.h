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
		uint32_t mAlbedoSId, mPositionId, mNormalId, mDepthStencilId;
	};

	struct PixelInfo
	{
		float ObjectId = 0.0f;
		float DrawId = 0.0f;
		float PrimitiveId = 0.0f;
	};

	class MousePickingCapture :public Framebuffer
	{
	public:
		MousePickingCapture(uint32_t w, uint32_t h);
		~MousePickingCapture();

		PixelInfo* ReadPixels(uint32_t xpos, uint32_t ypos);
	private:
		void Invalidate()override;
		uint32_t mPickingId;
		PixelInfo* mPixelInfo;
	};

	class DeferredPipeline :public Pipeline
	{
	public:
		void Init(uint32_t w, uint32_t h)override;
		void Visible(uint32_t w, uint32_t h)override;
		void Update(Scene* scene, Camera* camera)override;
		void PickedObject(int pickedobject)override;
		void MousePicking(bool value)override;
		void Shutdown()override;
	private:
		std::pair<MousePickingCapture*, Shader*> mMousePickingPass;
		std::pair<DeferredGeometryCapture*, Shader*> mGeometryPass;
		Shader* mOutline;
		Shader* mLightingPass;
		bool mMousePicking = true;
		int mPickedObjectId = -1;
	};
}