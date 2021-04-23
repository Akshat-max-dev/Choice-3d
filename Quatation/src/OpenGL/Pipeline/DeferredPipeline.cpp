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
		uint32_t tex[5] = { mPositionId, mNormalId, mAlbedoSId, mPickingId, mDepthStencilId };
		glDeleteTextures(5, tex);
	}

	void DeferredGeometryCapture::BindGBuffer(glm::uvec4 slots)
	{
		glActiveTexture(GL_TEXTURE0 + slots.x);
		glBindTexture(GL_TEXTURE_2D, mPositionId);
		glActiveTexture(GL_TEXTURE0 + slots.y);
		glBindTexture(GL_TEXTURE_2D, mNormalId);
		glActiveTexture(GL_TEXTURE0 + slots.z);
		glBindTexture(GL_TEXTURE_2D, mAlbedoSId);
		glActiveTexture(GL_TEXTURE0 + slots.w);
		glBindTexture(GL_TEXTURE_2D, mDepthStencilId);
	}

	PixelInfo* DeferredGeometryCapture::ReadPixels(uint32_t xpos, uint32_t ypos)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mRendererId);
		glReadBuffer(GL_COLOR_ATTACHMENT3);
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
			uint32_t tex[5] = { mPositionId, mNormalId, mAlbedoSId, mPickingId, mDepthStencilId };
			glDeleteTextures(5, tex);
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

		glCreateTextures(GL_TEXTURE_2D, 1, &mPickingId);
		glBindTexture(GL_TEXTURE_2D, mPickingId);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, mWidth, mHeight);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, mPickingId, 0);

		glCreateTextures(GL_TEXTURE_2D, 1, &mDepthStencilId);
		glBindTexture(GL_TEXTURE_2D, mDepthStencilId);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, mWidth, mHeight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mDepthStencilId, 0);

		uint32_t Attachments[4] = { GL_COLOR_ATTACHMENT0,
									GL_COLOR_ATTACHMENT1,
									GL_COLOR_ATTACHMENT2,
									GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(4, Attachments);

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
		mGeometryPass.first = new DeferredGeometryCapture(w, h);
		mGeometryPass.second = new Shader("Choice/assets/shaders/DeferredGeometryPass.glsl");

		mOutline = new Shader("Choice/assets/shaders/Outline.glsl");

		mLightingPass.first = new DeferredLightingCapture(w, h);
		mLightingPass.second = new Shader("Choice/assets/shaders/DeferredLightingPass.glsl");
	}

	void DeferredPipeline::Visible(uint32_t w, uint32_t h)
	{
		mGeometryPass.first->Visible(w, h);
		mLightingPass.first->Visible(w, h);
	}

	void DeferredPipeline::Update(Scene* scene, Camera* camera)
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		if (Input::IsButtonPressed(Mouse::BUTTON1) &&
			!(Input::IsKeyPressed(Key::LEFTSHIFT) || Input::IsKeyPressed(Key::LEFTALT))
			&& mMousePicking)
		{
			PixelInfo* pixelinfo = mGeometryPass.first->ReadPixels((uint32_t)Input::GetMouseX(),
				Choice::Instance()->GetWindow()->GetHeight() - (uint32_t)Input::GetMouseY() - 1);
			if (pixelinfo->PrimitiveId)
			{
				mPickedObjectId = static_cast<int>(pixelinfo->ObjectId);
			}
			else { mPickedObjectId = -1; }
			Choice::Instance()->GetEditor()->SetSelectedObjectIndex(mPickedObjectId);
		}

		mGeometryPass.first->Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		int objectindex = -1;
		for (auto& object : scene->GetSceneObjects())
		{
			objectindex++;
			if (object)
			{
				Model* model = object->GetProperty<Model>();
				if (model)
				{
					if (objectindex == mPickedObjectId)
					{
						glClear(GL_STENCIL_BUFFER_BIT);
						glStencilFunc(GL_ALWAYS, 1, 0xFF);
						glStencilMask(0xFF);
					}
					else
					{
						glStencilMask(0x00);
					}
					int drawindex = -1;
					for (auto& mesh : model->Meshes)
					{
						drawindex++;
						mGeometryPass.second->Use();
						if (model->Materials[mesh.second]->DiffuseMap)
						{
							model->Materials[mesh.second]->DiffuseMap->Bind(0);
							mGeometryPass.second->Int("gHasDiffuseMap", 1);
						}
						if (model->Materials[mesh.second]->NormalMap)
						{
							model->Materials[mesh.second]->NormalMap->Bind(1);
							mGeometryPass.second->Int("gHasNormalMap", 1);
						}
						mGeometryPass.second->Int("gMaterial.Diffuse", 0);
						mGeometryPass.second->Int("gMaterial.Normal", 1);
						mGeometryPass.second->Int("gObjectIndex", objectindex);
						mGeometryPass.second->Int("gDrawIndex", drawindex);
						mGeometryPass.second->Mat4("uViewProjection", camera->ViewProjection());
						mGeometryPass.second->Mat4("uTransform", object->GetProperty<Transform>()->GetTransform());
						mesh.first->Bind();
						uint32_t count = mesh.first->GetCount();
						glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
					}
				}

				Primitive* primitive = object->GetProperty<Primitive>();
				if (primitive)
				{
					if (objectindex == mPickedObjectId)
					{
						glClear(GL_STENCIL_BUFFER_BIT);
						glStencilFunc(GL_ALWAYS, 1, 0xFF);
						glStencilMask(0xFF);
					}
					else
					{
						glStencilMask(0x00);
					}
					mGeometryPass.second->Use();
					mGeometryPass.second->Int("gHasDiffuseMap", 0);
					mGeometryPass.second->Int("gHasNormalMap", 0);
					mGeometryPass.second->Int("gObjectIndex", objectindex);
					mGeometryPass.second->Int("gDrawIndex", -1);
					mGeometryPass.second->Mat4("uViewProjection", camera->ViewProjection());
					mGeometryPass.second->Mat4("uTransform", object->GetProperty<Transform>()->GetTransform());
					primitive->Draw();
				}
			}
		}
		mGeometryPass.first->UnBind();

		BlitData();

		glDepthMask(0x00);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);

		mLightingPass.first->Bind();
		glClear(GL_COLOR_BUFFER_BIT);
		int directinalLightCount = -1;
		int pointLightCount = -1;
		for (auto& object : scene->GetSceneObjects())
		{
			if (object)
			{
				Light* light = object->GetProperty<Light>();
				if (light)
				{
					Transform* transform = object->GetProperty<Transform>();
					mLightingPass.second->Use();
					mGeometryPass.first->BindGBuffer({ 0, 1, 2, 3 });
					mLightingPass.second->Int("lGBuffer.Position", 0);
					mLightingPass.second->Int("lGBuffer.Normal", 1);
					mLightingPass.second->Int("lGBuffer.AlbedoS", 2);
					switch (light->GetLightType())
					{
					case LightType::DIRECTIONAL:
						directinalLightCount++;
						mLightingPass.second->Float3(("ldLights[" + std::to_string(directinalLightCount) + "].Direction").c_str(), transform->GetTransform()[2]);
						mLightingPass.second->Float3(("ldLights[" + std::to_string(directinalLightCount) + "].Diffuse").c_str(), light->GetDiffuse()* light->GetIntensity());
						mLightingPass.second->Float3(("ldLights[" + std::to_string(directinalLightCount) + "].Specular").c_str(), light->GetDiffuse()* light->GetIntensity());
						break;
					case LightType::POINT:
						pointLightCount++;
						mLightingPass.second->Float3(("lpLights[" + std::to_string(pointLightCount) + "].Position").c_str(), transform->Position);
						mLightingPass.second->Float3(("lpLights[" + std::to_string(pointLightCount) + "].Diffuse").c_str(), light->GetDiffuse()* light->GetIntensity());
						mLightingPass.second->Float3(("lpLights[" + std::to_string(pointLightCount) + "].Specular").c_str(), light->GetDiffuse()* light->GetIntensity());
						mLightingPass.second->Float(("lpLights[" + std::to_string(pointLightCount) + "].Radius").c_str(), light->GetRadius() / 10.0f);
						break;
					}
					mLightingPass.second->Float3("lViewpos", camera->Position());
					mLightingPass.second->Int("ldLightsActive", directinalLightCount + 1);
					mLightingPass.second->Int("lpLightsActive", pointLightCount + 1);
				}
			}
		}
		glDrawArrays(GL_TRIANGLES, 0, 3);

		for (auto& object : scene->GetSceneObjects())
		{
			if (object)
			{
				Skybox* skybox = object->GetProperty<Skybox>();
				if (skybox)
				{
					glDisable(GL_STENCIL_TEST);
					glEnable(GL_DEPTH_TEST);
					glDepthFunc(GL_LEQUAL);
					skybox->Draw(camera);
					glDepthFunc(GL_LESS);
				}
			}
		}

		if (mPickedObjectId != -1)
		{
			SceneObject* object = scene->GetSceneObjects()[mPickedObjectId];
			if (object)
			{
				Model* model = object->GetProperty<Model>();
				if (model)
				{
					glEnable(GL_STENCIL_TEST);
					glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
					glStencilMask(0x00);
					glDisable(GL_DEPTH_TEST);
					for (auto& mesh : model->Meshes)
					{
						mOutline->Use();
						mOutline->Mat4("uProjection", camera->Projection());
						mOutline->Mat4("uViewProjection", camera->ViewProjection());
						Transform* transform = object->GetProperty<Transform>();
						mOutline->Mat4("uTransform", glm::scale(transform->GetTransform(), glm::vec3(1.02f, 1.02f, 1.02f)));
						mesh.first->Bind();
						uint32_t count = mesh.first->GetCount();
						glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
					}
				}

				Primitive* primitive = object->GetProperty<Primitive>();
				if (primitive)
				{
					glEnable(GL_STENCIL_TEST);
					glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
					glStencilMask(0x00);
					glDisable(GL_DEPTH_TEST);
					mOutline->Use();
					mOutline->Mat4("uProjection", camera->Projection());
					mOutline->Mat4("uViewProjection", camera->ViewProjection());
					Transform* transform = object->GetProperty<Transform>();
					mOutline->Mat4("uTransform", glm::scale(transform->GetTransform(), glm::vec3(1.02f, 1.02f, 1.02f)));
					primitive->Draw();
				}

				glStencilFunc(GL_ALWAYS, 0, 0xFF);
			}
			
		}
		mLightingPass.first->UnBind();

		glStencilMask(0xFF);
		glDepthMask(0x01);
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
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

}