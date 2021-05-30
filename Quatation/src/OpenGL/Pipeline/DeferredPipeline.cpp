#include "DeferredPipeline.h"

#include <glad/glad.h>
#include "Input.h"
#include "Error.h"

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
			Message<ERROR_MSG>("Geometry Pass Framebuffer Incomplete!", MESSAGE_ORIGIN::PIPELINE);
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

#ifdef DEBUG
		ReflectionData& reflectiondata = global::GlobalReflectionData;
		for (auto& sampler : reflectiondata.Samplers)
		{
			std::string msg = "Sampler[" + sampler.first + "] =";
			msg += " " + std::to_string(sampler.second);
			Message<INFO>(msg.c_str(), MESSAGE_ORIGIN::PIPELINE);
		}

		for (auto& buffer : reflectiondata.UniformBuffers)
		{
			std::string msg = "Uniform Buffer[" + buffer.first + "]";
			Message<INFO>(msg.c_str(), MESSAGE_ORIGIN::PIPELINE);
		}
#endif 
		global::CameraBuffer.resize(global::GlobalReflectionData.UniformBuffers["Camera"]->GetBufferSize());
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
		ReflectionData& reflectiondata = global::GlobalReflectionData;

		int directionallightcount = 0;
		int pointlightcount = 0;

		glm::mat4* projection = reflectiondata.UniformBuffers["Camera"]->MemberData<glm::mat4>("Camera.Projection", global::CameraBuffer);
		glm::mat4* view = reflectiondata.UniformBuffers["Camera"]->MemberData<glm::mat4>("Camera.View", global::CameraBuffer);
		glm::mat4* viewinv = reflectiondata.UniformBuffers["Camera"]->MemberData<glm::mat4>("Camera.ViewInv", global::CameraBuffer);
		glm::mat4* projectionInv = reflectiondata.UniformBuffers["Camera"]->MemberData<glm::mat4>("Camera.ProjectionInv", global::CameraBuffer);
		glm::vec3* camerapos = reflectiondata.UniformBuffers["Camera"]->MemberData<glm::vec3>("Camera.Position", global::CameraBuffer);

		*camerapos = camera->Position();
		*viewinv = glm::inverse(camera->View());
		*projectionInv = glm::inverse(camera->Projection());
		*projection = camera->Projection();
		*view = camera->View();

		reflectiondata.UniformBuffers["Camera"]->SetData(global::CameraBuffer);

		auto func = [&](Node* node) {
			switch (node->node_data_type)
			{
			case NODE_DATA_TYPE::NONE:
				UpdateWorldTransform(node);
				break;
			case NODE_DATA_TYPE::MESH:
				{
					Mesh* mesh = static_cast<Mesh*>(node);
					UpdateWorldTransform(mesh);
					reflectiondata.UniformBuffers["Transform"]->SetData("Transform.uTransform", &mesh->WorldTransform);

					auto& materialBuffer = reflectiondata.UniformBuffers["Material"];

					for (auto& primitive : mesh->primitives)
					{
						materialBuffer->SetData(primitive->material->Data);
						int* hasdisplacementmap = materialBuffer->MemberData<int>("Material.HasDisplacementMap", primitive->material->Data);
						reflectiondata.UniformBuffers["DisplacementMap"]->SetData("DisplacementMap.HasDisplacementMap", hasdisplacementmap);

						for (auto&& [type, texturemap] : primitive->material->TextureMaps)
						{
							if (texturemap->texture)
								texturemap->texture->Bind(GetBinding(type));
						}

						DrawPrimitive(primitive, mesh->mesh_type);
					}

					//Calculating Scene AABB
					glm::vec4 min = mesh->NodeTransform->GetTransform() * glm::vec4(mesh->boundingbox.Min, 1.0f);
					scene->GetBoundingBox().Min.x = min.x < scene->GetBoundingBox().Min.x ? min.x : scene->GetBoundingBox().Min.x;
					scene->GetBoundingBox().Min.y = min.y < scene->GetBoundingBox().Min.y ? min.y : scene->GetBoundingBox().Min.y;
					scene->GetBoundingBox().Min.z = min.z < scene->GetBoundingBox().Min.z ? min.z : scene->GetBoundingBox().Min.z;

					glm::vec4 max = mesh->NodeTransform->GetTransform() * glm::vec4(mesh->boundingbox.Max, 1.0f);
					scene->GetBoundingBox().Max.x = max.x > scene->GetBoundingBox().Max.x ? max.x : scene->GetBoundingBox().Max.x;
					scene->GetBoundingBox().Max.y = max.y > scene->GetBoundingBox().Max.y ? max.y : scene->GetBoundingBox().Max.y;
					scene->GetBoundingBox().Max.z = max.z > scene->GetBoundingBox().Max.z ? max.z : scene->GetBoundingBox().Max.z;
					break;
				}
			case NODE_DATA_TYPE::LIGHT:
				{
					Light* light = static_cast<Light*>(node);

					auto& lightBuffer = reflectiondata.UniformBuffers["Lights"];

					int* directionallightactive = lightBuffer->MemberData<int>("Lights.ldLightsActive", global::LightsBuffer);
					int* pointlightactive = lightBuffer->MemberData<int>("Lights.lpLightsActive", global::LightsBuffer);

					switch (light->Type)
					{
					case LIGHT_TYPE::DIRECTIONAL:
						directionallightcount++;
						{
							glm::vec3* direction = lightBuffer->MemberData<glm::vec3>("Lights.ldLights.Direction", light->Data);
							glm::mat4* lightVP = lightBuffer->MemberData<glm::mat4>("Lights.ldLights.LightVP", light->Data);

							*direction = glm::normalize(glm::vec3(node->NodeTransform->GetTransform()[2]));
							*lightVP = light->ViewProjection(&scene->GetBoundingBox())[0];

							auto* lightdata = lightBuffer->MemberData<char*>(("Lights.ldLights[" + std::to_string(directionallightcount - 1) + "]").c_str(), global::LightsBuffer);
							memcpy(lightdata, light->Data.data(), light->Data.size());
						}
						break;
					case LIGHT_TYPE::POINT:
						pointlightcount++;
						{
							auto* position = lightBuffer->MemberData<glm::vec3>("Lights.lpLights.Position", light->Data);
							*position = node->NodeTransform->Position;

							auto* lightdata = lightBuffer->MemberData<char*>(("Lights.lpLights[" + std::to_string(pointlightcount - 1) + "]").c_str(), global::LightsBuffer);
							memcpy(lightdata, light->Data.data(), light->Data.size());
						}
						break;
					}
					*directionallightactive = directionallightcount;
					*pointlightactive = pointlightcount;
					break;
				}
			}
		};

		glEnable(GL_DEPTH_TEST);

		mGeometryPass.first->Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		mGeometryPass.second->Use();
		for (auto& node : scene->GetNodes())
		{
			IterateNodes(node, func);
		}
		mGeometryPass.first->UnBind();

		//TODO : Add Shadow Map Pass

		BlitData();

		glDisable(GL_DEPTH_TEST);

		//Bind All Textures To Be Used In Lighting Pass
		auto& samplers = reflectiondata.Samplers;

		uint32_t slots[] = { samplers["lNormal"], samplers["lAlbedoS"],
							 samplers["lRoughMetalAo"], samplers["lDepthMap"] };

		mGeometryPass.first->BindGBuffer(slots);
		scene->GetSkybox()->BindIBL({ samplers["lIrradianceMap"], samplers["lPreFilterMap"], samplers["lBRDFLookup"] });
		mShadowMapPass.first->BindShadowMap(samplers["lShadowMap"]);

		mLightingPass.first->Bind();
		mLightingPass.second->Use();
		reflectiondata.UniformBuffers["Lights"]->SetData(global::LightsBuffer);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		//Draw Skybox
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		scene->GetSkybox()->Draw(camera);
		glDepthFunc(GL_LESS);

		mLightingPass.first->UnBind();

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
			Message<ERROR_MSG>("Lighting Pass Framebuffer Incomplete!", MESSAGE_ORIGIN::PIPELINE);
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
			Message<ERROR_MSG>("Shadow Map Pass Framebuffer Incomplete!", MESSAGE_ORIGIN::PIPELINE);
#endif
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}