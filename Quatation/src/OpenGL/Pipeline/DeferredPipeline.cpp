#include "DeferredPipeline.h"

#include <glad/glad.h>
#include "Input.h"
#include "Choice.h"

namespace choice
{
	DeferredGeometryCapture::DeferredGeometryCapture(uint32_t w, uint32_t h)
		:Framebuffer(w, h)
	{
		mPixelInfo = new PixelInfo();
		Invalidate();
	}

	DeferredGeometryCapture::~DeferredGeometryCapture()
	{
		delete mPixelInfo;
		Framebuffer::Destroy();
		uint32_t tex[6] = { mPositionId, mNormalId, mAlbedoSId, mRoughMetalAo, mPickingId, mDepthStencilId };
		glDeleteTextures(6, tex);
	}

	void DeferredGeometryCapture::BindGBuffer(uint32_t slots[])
	{
		glActiveTexture(GL_TEXTURE0 + slots[0]);
		glBindTexture(GL_TEXTURE_2D, mPositionId);
		glActiveTexture(GL_TEXTURE0 + slots[1]);
		glBindTexture(GL_TEXTURE_2D, mNormalId);
		glActiveTexture(GL_TEXTURE0 + slots[2]);
		glBindTexture(GL_TEXTURE_2D, mAlbedoSId);
		glActiveTexture(GL_TEXTURE0 + slots[3]);
		glBindTexture(GL_TEXTURE_2D, mRoughMetalAo);
	}

	PixelInfo* DeferredGeometryCapture::ReadPixels(uint32_t xpos, uint32_t ypos)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mRendererId);
		glReadBuffer(GL_COLOR_ATTACHMENT4);
		glReadPixels(static_cast<GLint>(xpos), static_cast<GLint>(ypos), 1, 1, GL_RGB, GL_FLOAT, mPixelInfo);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return mPixelInfo;
	}

	void DeferredGeometryCapture::Invalidate()
	{
		if (mRendererId)
		{
			Framebuffer::Destroy();
			uint32_t tex[6] = { mPositionId, mNormalId, mAlbedoSId, mRoughMetalAo, mPickingId, mDepthStencilId };
			glDeleteTextures(6, tex);
		}

		glCreateFramebuffers(1, &mRendererId);
		glBindFramebuffer(GL_FRAMEBUFFER, mRendererId);

		glCreateTextures(GL_TEXTURE_2D, 1, &mPositionId);
		glBindTexture(GL_TEXTURE_2D, mPositionId);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, mWidth, mHeight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mPositionId, 0);

		glCreateTextures(GL_TEXTURE_2D, 1, &mNormalId);
		glBindTexture(GL_TEXTURE_2D, mNormalId);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, mWidth, mHeight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mNormalId, 0);

		glCreateTextures(GL_TEXTURE_2D, 1, &mAlbedoSId);
		glBindTexture(GL_TEXTURE_2D, mAlbedoSId);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, mWidth, mHeight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, mAlbedoSId, 0);

		glCreateTextures(GL_TEXTURE_2D, 1, &mRoughMetalAo);
		glBindTexture(GL_TEXTURE_2D, mRoughMetalAo);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, mWidth, mHeight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, mRoughMetalAo, 0);

		glCreateTextures(GL_TEXTURE_2D, 1, &mPickingId);
		glBindTexture(GL_TEXTURE_2D, mPickingId);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, mWidth, mHeight);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, mPickingId, 0);

		glCreateTextures(GL_TEXTURE_2D, 1, &mDepthStencilId);
		glBindTexture(GL_TEXTURE_2D, mDepthStencilId);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, mWidth, mHeight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mDepthStencilId, 0);

		uint32_t Attachments[5] = { GL_COLOR_ATTACHMENT0,
									GL_COLOR_ATTACHMENT1,
									GL_COLOR_ATTACHMENT2,
									GL_COLOR_ATTACHMENT3,
									GL_COLOR_ATTACHMENT4 };
		glDrawBuffers(5, Attachments);

		if (!(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE))
		{
#ifdef DEBUG
			std::cout << "Framebuffer Not Complete" << std::endl;
			choiceassert(0);
#endif
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void DeferredPipeline::Init(uint32_t w, uint32_t h)
	{
		//Configure Geometry Pass
		mGeometryPass.first = new DeferredGeometryCapture(w, h);
		mGeometryPass.second = new Shader("Choice/assets/shaders/DeferredGeometryPass.glsl");

		//Configure Samplers
		mGeometryPass.second->Use();
		mGeometryPass.second->Int("gMaterial.Diffuse", 0);
		mGeometryPass.second->Int("gMaterial.Normal", 1);
		mGeometryPass.second->Int("gMaterial.Roughness", 2);
		mGeometryPass.second->Int("gMaterial.Metallic", 3);
		mGeometryPass.second->Int("gMaterial.AmbientOcclusion", 4);

		//Configure Shadow Map Pass
		mShadowMapPass.first = new ShadowMapCapture(1024, 1024);
		mShadowMapPass.second = new Shader("Choice/assets/shaders/ShadowMap.glsl");

		//Configure Outline Pass
		mOutline = new Shader("Choice/assets/shaders/Line.glsl");

		//Configure Lighting Pass
		mLightingPass.first = new DeferredLightingCapture(w, h);
		mLightingPass.second = new Shader("Choice/assets/shaders/DeferredLightingPass.glsl");

		//Configure Samplers
		mLightingPass.second->Use();
		mLightingPass.second->Int("lGBuffer.Position", 0);
		mLightingPass.second->Int("lGBuffer.Normal", 1);
		mLightingPass.second->Int("lGBuffer.AlbedoS", 2);
		mLightingPass.second->Int("lGBuffer.RoughMetalAo", 3);
		mLightingPass.second->Int("lIBL.IrradianceMap", 4);
		mLightingPass.second->Int("lIBL.PreFilterMap", 5);
		mLightingPass.second->Int("lIBL.BRDFLookup", 6);
		mLightingPass.second->Int("lShadowMap", 7);
	}

	void DeferredPipeline::Visible(uint32_t w, uint32_t h)
	{
		mGeometryPass.first->Visible(w, h);
		mLightingPass.first->Visible(w, h);
	}

	static void DrawPrimitive(Primitive* primitive, MESH_TYPE& type)
	{
		primitive->vertexarray->Bind();
		uint32_t count = primitive->vertexarray->GetCount();
		switch (type)
		{
		case MESH_TYPE::CUBE:
			glDrawArrays(GL_TRIANGLES, 0, 36);
			break;
		case MESH_TYPE::SPHERE:
			glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, nullptr);
			break;
		default:
			glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
			break;
		}
	}

	static void IterateNodes(Node* node, const std::function<void(Node*)>& func)
	{
		if (node)
		{
			func(node);

			//Children
			for (auto& child : node->Children)
			{
				IterateNodes(child, func);
			}
		}
	}

	void DeferredPipeline::Update(Scene* scene, Camera* camera)
	{
		glEnable(GL_DEPTH_TEST);

		//Geometry Pass
		mGeometryPass.first->Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		mGeometryPass.second->Use();

		for (auto& node : scene->GetNodes())
		{
			mGeometryPass.second->Mat4("uViewProjection", camera->ViewProjection());
			mGeometryPass.second->Int("gNodeIndex", 0);
			mGeometryPass.second->Int("gDrawIndex", -1);
			mGeometryPass.second->Float4("gMaterial.Color", { 1.0f, 0.2f, 0.3f, 1.0f });
			mGeometryPass.second->Float("gMaterial.RoughnessFactor", 1.0f);
			mGeometryPass.second->Float("gMaterial.MetallicFactor", 0.0f);
			mGeometryPass.second->Float("gMaterial.AO", 1.0f);
			mGeometryPass.second->Int("gHasDiffuseMap", 0);
			mGeometryPass.second->Int("gHasNormalMap", 0);
			mGeometryPass.second->Int("gHasRoughnessMap", 0);
			mGeometryPass.second->Int("gHasMetallicMap", 0);
			mGeometryPass.second->Int("gHasAmbientOcclusionMap", 0);
			auto func = [&](Node* node) {
				if (node->node_data_type == NODE_DATA_TYPE::MESH)
				{
					Mesh* mesh = static_cast<Mesh*>(node);
					if (mesh->Parent)
					{
						mesh->WorldTransform = mesh->Parent->WorldTransform * mesh->NodeTransform->GetTransform();
					}
					else
					{
						mesh->WorldTransform = mesh->NodeTransform->GetTransform();
					}
					mGeometryPass.second->Mat4("uTransform", mesh->WorldTransform);
					
					for (auto& primitive : mesh->primitives)
					{					
						DrawPrimitive(primitive, mesh->mesh_type);
					}

					//Calculating Scene AABB
					glm::vec4 min = mesh->NodeTransform->GetTransform() *
						glm::vec4(mesh->boundingbox.Min, 1.0f);
					scene->GetBoundingBox().Min.x = min.x < scene->GetBoundingBox().Min.x ? min.x : scene->GetBoundingBox().Min.x;
					scene->GetBoundingBox().Min.y = min.y < scene->GetBoundingBox().Min.y ? min.y : scene->GetBoundingBox().Min.y;
					scene->GetBoundingBox().Min.z = min.z < scene->GetBoundingBox().Min.z ? min.z : scene->GetBoundingBox().Min.z;

					glm::vec4 max = mesh->NodeTransform->GetTransform() *
						glm::vec4(mesh->boundingbox.Max, 1.0f);
					scene->GetBoundingBox().Max.x = max.x > scene->GetBoundingBox().Max.x ? max.x : scene->GetBoundingBox().Max.x;
					scene->GetBoundingBox().Max.y = max.y > scene->GetBoundingBox().Max.y ? max.y : scene->GetBoundingBox().Max.y;
					scene->GetBoundingBox().Max.z = max.z > scene->GetBoundingBox().Max.z ? max.z : scene->GetBoundingBox().Max.z;
				}
			};
			IterateNodes(node, func);
		}
		mGeometryPass.first->UnBind();

		//Shadow Map Pass
		mShadowMapPass.first->Bind();
		glClear(GL_DEPTH_BUFFER_BIT);
		mShadowMapPass.second->Use();

		for (auto& node : scene->GetNodes())
		{
			auto func = [&](Node* node) {
				if (node->node_data_type == NODE_DATA_TYPE::LIGHT)
				{
					Light* light = static_cast<Light*>(node);
					switch (light->Type)
					{
					case LIGHT_TYPE::DIRECTIONAL:
						mShadowMapPass.second->Mat4("uLightViewProjection", light->ViewProjection(&scene->GetBoundingBox())[0]);
						for (auto& mesh_node : scene->GetNodes())
						{
							auto mesh_func = [=](Node* node) {
								if (node->node_data_type == NODE_DATA_TYPE::MESH)
								{
									Mesh* mesh = static_cast<Mesh*>(node);
									if (mesh->Parent)
									{
										mesh->WorldTransform = mesh->Parent->WorldTransform * mesh->NodeTransform->GetTransform();
									}
									else
									{
										mesh->WorldTransform = mesh->NodeTransform->GetTransform();
									}
									mGeometryPass.second->Mat4("uTransform", mesh->WorldTransform);
									for (auto& primitive : mesh->primitives)
									{
										DrawPrimitive(primitive, mesh->mesh_type);
									}
								}
							};
							IterateNodes(mesh_node, mesh_func);
						}
						break;
					}
				}
			};
			IterateNodes(node, func);
		}

		mShadowMapPass.first->UnBind();

		BlitData();

		glDepthMask(0x00);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);

		//Lighting Pass
		mLightingPass.first->Bind();
		glClear(GL_COLOR_BUFFER_BIT);
		int directinalLightCount = -1;
		int pointLightCount = -1;
		for (auto& node : scene->GetNodes())
		{
			auto func = [&](Node* node) {
				if (node->node_data_type == NODE_DATA_TYPE::LIGHT)
				{
					Light* light = static_cast<Light*>(node);
					mLightingPass.second->Use();
					uint32_t slots[] = { 0, 1, 2, 3 };
					mGeometryPass.first->BindGBuffer(slots);
					scene->GetSkybox()->BindIBL({ 4, 5, 6 });
					mShadowMapPass.first->BindShadowMap(7);
					switch (light->Type)
					{
					case LIGHT_TYPE::DIRECTIONAL:
						directinalLightCount++;
						mLightingPass.second->Mat4("uLightViewProjection", light->ViewProjection(&scene->GetBoundingBox())[0]);
						mLightingPass.second->Float3(("ldLights[" + std::to_string(directinalLightCount) + "].Direction").c_str(), glm::normalize(node->NodeTransform->GetTransform()[2]));
						mLightingPass.second->Float3(("ldLights[" + std::to_string(directinalLightCount) + "].Diffuse").c_str(), light->Color * light->Intensity);
						mLightingPass.second->Float3(("ldLights[" + std::to_string(directinalLightCount) + "].Specular").c_str(), light->Color * light->Intensity);
						break;
					case LIGHT_TYPE::POINT:
						pointLightCount++;
						mLightingPass.second->Float3(("lpLights[" + std::to_string(pointLightCount) + "].Position").c_str(), node->NodeTransform->Position);
						mLightingPass.second->Float3(("lpLights[" + std::to_string(pointLightCount) + "].Diffuse").c_str(), light->Color * light->Intensity);
						mLightingPass.second->Float3(("lpLights[" + std::to_string(pointLightCount) + "].Specular").c_str(), light->Color * light->Intensity);
						mLightingPass.second->Float(("lpLights[" + std::to_string(pointLightCount) + "].Radius").c_str(), light->Radius / 10.0f);
						break;
					}
					mLightingPass.second->Float3("lViewpos", camera->Position());
					mLightingPass.second->Int("ldLightsActive", directinalLightCount + 1);
					mLightingPass.second->Int("lpLightsActive", pointLightCount + 1);
				}
			};
			IterateNodes(node, func);
		}
		glDrawArrays(GL_TRIANGLES, 0, 3);

		//Draw Skybox
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		scene->GetSkybox()->Draw(camera);
		glDepthFunc(GL_LESS);

		mLightingPass.first->UnBind();

		glDepthMask(0x01);

		//Reset Scene AABB
		scene->GetBoundingBox() = CalculateBoundingBox(nullptr, 0, 0);
	}

	void DeferredPipeline::BlitData()
	{
		mGeometryPass.first->BindRead();
		mLightingPass.first->BindDraw();
		glBlitFramebuffer(0, 0, mGeometryPass.first->GetWidth(), mGeometryPass.first->GetHeight(),
			0, 0, mLightingPass.first->GetWidth(), mLightingPass.first->GetHeight(),
			GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	const uint32_t& DeferredPipeline::Capture() const
	{
		return mLightingPass.first->GetCapture();
	}

	void DeferredPipeline::Shutdown()
	{
		delete mGeometryPass.first;
		delete mGeometryPass.second;
		delete mShadowMapPass.first;
		delete mShadowMapPass.second;
		delete mOutline;
		delete mLightingPass.first;
		delete mLightingPass.second;
	}

	DeferredLightingCapture::DeferredLightingCapture(uint32_t w, uint32_t h)
		:Framebuffer(w, h)
	{
		Invalidate();
	}

	DeferredLightingCapture::~DeferredLightingCapture()
	{
		Framebuffer::Destroy();
		glDeleteTextures(1, &mCapture);
		glDeleteTextures(1, &mDepthStencilId);
	}

	const uint32_t& DeferredLightingCapture::GetCapture() const
	{
		return mCapture;
	}

	void DeferredLightingCapture::Invalidate()
	{
		if (mRendererId)
		{
			Framebuffer::Destroy();
			glDeleteTextures(1, &mCapture);
			glDeleteTextures(1, &mDepthStencilId);
		}

		glCreateFramebuffers(1, &mRendererId);
		glBindFramebuffer(GL_FRAMEBUFFER, mRendererId);

		glCreateTextures(GL_TEXTURE_2D, 1, &mCapture);
		glBindTexture(GL_TEXTURE_2D, mCapture);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, mWidth, mHeight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mCapture, 0);

		glCreateTextures(GL_TEXTURE_2D, 1, &mDepthStencilId);
		glBindTexture(GL_TEXTURE_2D, mDepthStencilId);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, mWidth, mHeight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mDepthStencilId, 0);

		if (!(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE))
		{
#ifdef DEBUG
			std::cout << "Framebuffer Not Complete" << std::endl;
			choiceassert(0);
#endif
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	ShadowMapCapture::ShadowMapCapture(uint32_t w, uint32_t h)
		:Framebuffer(w, h)
	{
		Invalidate();
	}

	ShadowMapCapture::~ShadowMapCapture()
	{
		Framebuffer::Destroy();
		glDeleteTextures(1, &mShadowmap);
	}

	void ShadowMapCapture::BindShadowMap(uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, mShadowmap);
	}

	void ShadowMapCapture::Invalidate()
	{
		if (mRendererId)
		{
			Framebuffer::Destroy();
			glDeleteTextures(1, &mShadowmap);
		}

		glCreateFramebuffers(1, &mRendererId);
		glBindFramebuffer(GL_FRAMEBUFFER, mRendererId);

		glCreateTextures(GL_TEXTURE_2D, 1, &mShadowmap);
		glBindTexture(GL_TEXTURE_2D, mShadowmap);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, mWidth, mHeight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mShadowmap, 0);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		if (!(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE))
		{
#ifdef DEBUG
			std::cout << "Framebuffer Not Complete" << std::endl;
			choiceassert(0);
#endif
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

}