#include "DeferredPipeline.h"

#include <glad/glad.h>
#include "Input.h"
#include "Choice.h"

namespace choice
{
	DeferredGeometryCapture::DeferredGeometryCapture(uint32_t w, uint32_t h)
		:Framebuffer(w, h)
	{
		Invalidate();
	}

	DeferredGeometryCapture::~DeferredGeometryCapture()
	{
		Framebuffer::Destroy();
		uint32_t tex[4] = { mNormalId, mAlbedoSId, mRoughMetalAo, mDepthId };
		glDeleteTextures(4, tex);
	}

	void DeferredGeometryCapture::BindGBuffer(uint32_t slots[])const
	{
		glBindTextureUnit(slots[0], mNormalId);
		glBindTextureUnit(slots[1], mAlbedoSId);
		glBindTextureUnit(slots[2], mRoughMetalAo);
		glBindTextureUnit(slots[3], mDepthId);
	}

	void DeferredGeometryCapture::Invalidate()
	{
		if (mRendererId)
		{
			Framebuffer::Destroy();
			uint32_t tex[4] = { mNormalId, mAlbedoSId, mRoughMetalAo, mDepthId };
			glDeleteTextures(4, tex);
		}

		glCreateFramebuffers(1, &mRendererId);
		glBindFramebuffer(GL_FRAMEBUFFER, mRendererId);

		glCreateTextures(GL_TEXTURE_2D, 1, &mNormalId);
		glBindTexture(GL_TEXTURE_2D, mNormalId);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, mWidth, mHeight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mNormalId, 0);

		glCreateTextures(GL_TEXTURE_2D, 1, &mAlbedoSId);
		glBindTexture(GL_TEXTURE_2D, mAlbedoSId);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, mWidth, mHeight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mAlbedoSId, 0);

		glCreateTextures(GL_TEXTURE_2D, 1, &mRoughMetalAo);
		glBindTexture(GL_TEXTURE_2D, mRoughMetalAo);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, mWidth, mHeight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, mRoughMetalAo, 0);

		glCreateTextures(GL_TEXTURE_2D, 1, &mDepthId);
		glBindTexture(GL_TEXTURE_2D, mDepthId);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, mWidth, mHeight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthId, 0);

		uint32_t Attachments[3] = { GL_COLOR_ATTACHMENT0,
									GL_COLOR_ATTACHMENT1,
									GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, Attachments);

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

		//Configure Shadow Map Pass
		mShadowMapPass.first = new ShadowMapCapture(1024, 1024);
		mShadowMapPass.second = new Shader("Choice/assets/shaders/ShadowMap.glsl");

		//Configure Outline Pass
		mColor = new Shader("Choice/assets/shaders/Color.glsl");

		//Configure Lighting Pass
		mLightingPass.first = new DeferredLightingCapture(w, h);
		mLightingPass.second = new Shader("Choice/assets/shaders/DeferredLightingPass.glsl");

		global::CaptureBuffer.resize(global::GlobalReflectionData.UniformBuffers["Capture"]->GetBufferSize());
		global::LightsBuffer.resize(global::GlobalReflectionData.UniformBuffers["Lights"]->GetBufferSize());
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

	void DeferredPipeline::Update(Scene* scene, Camera* camera)
	{
		glEnable(GL_DEPTH_TEST);

		ReflectionData& reflectiondata = global::GlobalReflectionData;

		reflectiondata.UniformBuffers["Camera"]->SetData("Camera.uViewProjection", &camera->ViewProjection());

		//Geometry Pass
		mGeometryPass.first->Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		mGeometryPass.second->Use();

		static auto geometrypassfunc = [&](Node* node) {
			if (node->node_data_type == NODE_DATA_TYPE::MESH)
			{
				Mesh* mesh = static_cast<Mesh*>(node);
				UpdateWorldTransform(mesh);
				reflectiondata.UniformBuffers["Transform"]->SetData("Transform.uTransform", &mesh->WorldTransform);

				auto* materialBuffer = reflectiondata.UniformBuffers["Material"];

				for (auto& primitive : mesh->primitives)
				{
					materialBuffer->SetData(primitive->material->Data);

					for (auto&& [type, texturemap] : primitive->material->TextureMaps)
					{
						if (texturemap->texture)
							texturemap->texture->Bind(GetBinding(type));
					}

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

		for (auto& node : scene->GetNodes())
		{
			IterateNodes(node, geometrypassfunc);
		}
		mGeometryPass.first->UnBind();

		//Shadow Map Pass
		mShadowMapPass.first->Bind();
		glClear(GL_DEPTH_BUFFER_BIT);
		mShadowMapPass.second->Use();

		static auto shadowmappassfunc = [&](Node* node) {
			if (node->node_data_type == NODE_DATA_TYPE::LIGHT)
			{
				Light* light = static_cast<Light*>(node);
				switch (light->Type)
				{
				case LIGHT_TYPE::DIRECTIONAL:
					reflectiondata.UniformBuffers["Camera"]->SetData("Camera.uViewProjection", &light->ViewProjection(&scene->GetBoundingBox())[0]);
					for (auto& mesh_node : scene->GetNodes())
					{
						auto mesh_func = [&](Node* node) {
							if (node->node_data_type == NODE_DATA_TYPE::MESH)
							{
								Mesh* mesh = static_cast<Mesh*>(node);
								UpdateWorldTransform(mesh);
								reflectiondata.UniformBuffers["Transform"]->SetData("Transform.uTransform", &mesh->WorldTransform);
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

		for (auto& node : scene->GetNodes())
		{
			IterateNodes(node, shadowmappassfunc);
		}

		mShadowMapPass.first->UnBind();

		BlitData();

		glDepthMask(0x00);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);

		//Bind All Textures To Be Used In Lighting Pass
		auto& samplers = reflectiondata.Samplers;

		uint32_t slots[] = { samplers["lNormal"], samplers["lAlbedoS"], 
							 samplers["lRoughMetalAo"], samplers["lDepthMap"] };

		mGeometryPass.first->BindGBuffer(slots);
		scene->GetSkybox()->BindIBL({ samplers["lIrradianceMap"], samplers["lPreFilterMap"], samplers["lBRDFLookup"] });
		mShadowMapPass.first->BindShadowMap(samplers["lShadowMap"]);

		glm::mat4* projinv = reflectiondata.UniformBuffers["Capture"]->MemberData<glm::mat4>("Capture.uProjection", global::CaptureBuffer);
		*projinv = glm::inverse(camera->Projection());
		glm::mat4* viewinv = reflectiondata.UniformBuffers["Capture"]->MemberData<glm::mat4>("Capture.uView", global::CaptureBuffer);
		*viewinv = glm::inverse(camera->View());

		reflectiondata.UniformBuffers["Capture"]->SetData(global::CaptureBuffer);

		//Lighting Pass
		mLightingPass.first->Bind();
		glClear(GL_COLOR_BUFFER_BIT);
		int directinalLightCount = 0;
		int pointLightCount = 0;

		static auto lightingpassfunc = [&](Node* node) {
			if (node->node_data_type == NODE_DATA_TYPE::LIGHT)
			{
				Light* light = static_cast<Light*>(node);
				mLightingPass.second->Use();

				auto* lightBuffer = reflectiondata.UniformBuffers["Lights"];

				int* directionallightactive = lightBuffer->MemberData<int>("Lights.ldLightsActive", global::LightsBuffer);
				int* pointlightactive = lightBuffer->MemberData<int>("Lights.lpLightsActive", global::LightsBuffer);
				glm::vec3* viewpos = lightBuffer->MemberData<glm::vec3>("Lights.lViewpos", global::LightsBuffer);

				switch (light->Type)
				{
				case LIGHT_TYPE::DIRECTIONAL:
					directinalLightCount++;
					{
						glm::vec3* direction = lightBuffer->MemberData<glm::vec3>("Lights.ldLights.Direction", light->Data);
						glm::mat4* lightVP = lightBuffer->MemberData<glm::mat4>("Lights.ldLights.LightVP", light->Data);

						*direction = glm::normalize(glm::vec3(node->NodeTransform->GetTransform()[2]));
						*lightVP = light->ViewProjection(&scene->GetBoundingBox())[0];

						auto* lightdata = lightBuffer->MemberData<char*>(("Lights.ldLights[" + std::to_string(directinalLightCount - 1) + "]").c_str(), global::LightsBuffer);
						memcpy(lightdata, light->Data.data(), light->Data.size());
					}
					break;
				case LIGHT_TYPE::POINT:
					pointLightCount++;
					{
						auto* position = lightBuffer->MemberData<glm::vec3>("Lights.lpLights.Position", light->Data);
						*position = node->NodeTransform->Position;

						auto* lightdata = lightBuffer->MemberData<char*>(("Lights.lpLights[" + std::to_string(directinalLightCount - 1) + "]").c_str(), global::LightsBuffer);
						memcpy(lightdata, light->Data.data(), light->Data.size());
					}
					break;
				}

				*directionallightactive = directinalLightCount;
				*pointlightactive = pointLightCount;
				*viewpos = camera->Position();

				lightBuffer->SetData(global::LightsBuffer);
			}
		};

		for (auto& node : scene->GetNodes())
		{
			IterateNodes(node, lightingpassfunc);
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
			GL_DEPTH_BUFFER_BIT, GL_NEAREST);
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
		delete mColor;
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
		glDeleteTextures(1, &mDepthId);
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
			glDeleteTextures(1, &mDepthId);
		}

		glCreateFramebuffers(1, &mRendererId);
		glBindFramebuffer(GL_FRAMEBUFFER, mRendererId);

		glCreateTextures(GL_TEXTURE_2D, 1, &mCapture);
		glBindTexture(GL_TEXTURE_2D, mCapture);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, mWidth, mHeight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mCapture, 0);

		glCreateTextures(GL_TEXTURE_2D, 1, &mDepthId);
		glBindTexture(GL_TEXTURE_2D, mDepthId);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, mWidth, mHeight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthId, 0);

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
		glBindTextureUnit(slot, mShadowmap);
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