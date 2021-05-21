#pragma once
#include "cpch.h"

#include "Pipeline.h"
#include "OpenGL/Framebuffer.h"
#include "OpenGL/Shader.h"

#include "Project/Scene/Scene.h"

#include <glm/glm.hpp>

namespace choice
{
	struct PixelInfo
	{
		float ObjectId = 0.0f;
		float DrawId = 0.0f;
		float PrimitiveId = 0.0f;
	};

	class DeferredGeometryCapture :public Framebuffer
	{
	public:
		DeferredGeometryCapture(uint32_t w, uint32_t h);
		~DeferredGeometryCapture();

		void BindGBuffer(uint32_t slots[]);
		PixelInfo* ReadPixels(uint32_t xpos, uint32_t ypos);
	private:
		void Invalidate()override;
		uint32_t mAlbedoSId, mPositionId, mNormalId, mRoughMetalAo, mPickingId, mDepthStencilId;
		PixelInfo* mPixelInfo;
	};

	class DeferredLightingCapture :public Framebuffer
	{
	public:
		DeferredLightingCapture(uint32_t w, uint32_t h);
		~DeferredLightingCapture();

		const uint32_t& GetCapture()const;
	private:
		void Invalidate()override;
		uint32_t mCapture, mDepthStencilId;
	};

	class ShadowMapCapture :public Framebuffer
	{
	public:
		ShadowMapCapture(uint32_t w, uint32_t h);
		~ShadowMapCapture();

		void BindShadowMap(uint32_t slot)const;
	private:
		void Invalidate()override;
		uint32_t mShadowmap;
	};

	class DeferredPipeline :public Pipeline
	{
	public:
		void Init(uint32_t w, uint32_t h)override;
		void Visible(uint32_t w, uint32_t h)override;
		void Update(Scene* scene, Camera* camera)override;
		void BlitData();//Copy Depth-Stencil Data Of Geometry Pass To Lighting Pass
		const uint32_t& Capture()const override;
		void Shutdown()override;
	private:
		std::pair<ShadowMapCapture*, Shader*> mShadowMapPass;
		std::pair<DeferredGeometryCapture*, Shader*> mGeometryPass;
		Shader* mColor;
		std::pair<DeferredLightingCapture*, Shader*> mLightingPass;

		ReflectionData mReflectionData;
	};
}