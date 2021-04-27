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
		glActiveTexture(GL_TEXTURE0 + slots[4]);
		glBindTexture(GL_TEXTURE_2D, mDepthStencilId);
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
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
		mGeometryPass.first = new DeferredGeometryCapture(w, h);
		mGeometryPass.second = new Shader("Choice/assets/shaders/DeferredGeometryPass.glsl");

		mOutline = new Shader("Choice/assets/shaders/Outline.glsl");

		mLightingPass.first = new DeferredLightingCapture(w, h);
		mLightingPass.second = new Shader("Choice/assets/shaders/DeferredLightingPass.glsl");
		//mLightingPass.second = new Shader("Choice/assets/shaders/PBR.glsl");
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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		int objectindex = -1;
		for (auto& object : scene->GetSceneObjects())
		{
			objectindex++;
			if (object)
			{
				Drawable* drawable = object->GetProperty<Drawable>();
				if (drawable)
				{
					glStencilFunc(GL_ALWAYS, 1, 0xFF);
					if (objectindex == mPickedObjectId)
					{
						glStencilMask(0xFF);
					}
					int drawindex = -1;
					mGeometryPass.second->Use();
					for (auto& mesh : drawable->GetMeshes())
					{
						drawindex++;
						//Bind Diffuse Map
						if (drawable->GetMaterials()[mesh.second]->DiffuseMap.first 
							&& drawable->GetMaterials()[mesh.second]->DiffuseMap.second.first)
						{
							drawable->GetMaterials()[mesh.second]->DiffuseMap.second.first->Bind(0);
							mGeometryPass.second->Int("gHasDiffuseMap", 1);
						}
						else
						{
							mGeometryPass.second->Float4("gMaterial.Color", drawable->GetMaterials()[mesh.second]->Color);
							mGeometryPass.second->Int("gHasDiffuseMap", 0);
						}

						//Bind Normal Map
						if (drawable->GetMaterials()[mesh.second]->NormalMap.first 
							&& drawable->GetMaterials()[mesh.second]->NormalMap.second.first)
						{
							drawable->GetMaterials()[mesh.second]->NormalMap.second.first->Bind(1);
							mGeometryPass.second->Int("gHasNormalMap", 1);
						}
						else
						{
							mGeometryPass.second->Int("gHasNormalMap", 0);
						}

						mGeometryPass.second->Int("gMaterial.Diffuse", 0);
						mGeometryPass.second->Int("gMaterial.Normal", 1);
						mGeometryPass.second->Float("gMaterial.Roughness", drawable->GetMaterials()[mesh.second]->Roughness);
						mGeometryPass.second->Float("gMaterial.Metallic", drawable->GetMaterials()[mesh.second]->Metallic);
						mGeometryPass.second->Float("gMaterial.AO", 1.0f);
						mGeometryPass.second->Int("gObjectIndex", objectindex);
						mGeometryPass.second->Int("gDrawIndex", drawindex);
						mGeometryPass.second->Mat4("uViewProjection", camera->ViewProjection());
						mGeometryPass.second->Mat4("uTransform", object->GetProperty<Transform>()->GetTransform());
						mesh.first->Bind();

						switch (drawable->GetDrawableType())
						{
						case DrawableType::CUBE:
							glDrawArrays(GL_TRIANGLES, 0, 36);
							break;
						case DrawableType::SPHERE:
							{
								uint32_t scount = mesh.first->GetCount();
								glDrawElements(GL_TRIANGLE_STRIP, scount, GL_UNSIGNED_INT, nullptr);
							}
							break;
						case DrawableType::MODEL:
							{
								uint32_t mcount = mesh.first->GetCount();
								glDrawElements(GL_TRIANGLES, mcount, GL_UNSIGNED_INT, nullptr);
							}
							break;
						}
					}
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
					uint32_t slots[] = { 0, 1, 2, 3, 4 };
					mGeometryPass.first->BindGBuffer(slots);
					scene->GetSceneObjects()[0]->GetProperty<Skybox>()->BindIBL({ 5, 6, 7 });
					mLightingPass.second->Int("lGBuffer.Position", 0);
					mLightingPass.second->Int("lGBuffer.Normal", 1);
					mLightingPass.second->Int("lGBuffer.AlbedoS", 2);
					mLightingPass.second->Int("lGBuffer.RoughMetalAo", 3);
					mLightingPass.second->Int("lIBL.IrradianceMap", 5);
					mLightingPass.second->Int("lIBL.PreFilterMap", 6);
					mLightingPass.second->Int("lIBL.BRDFLookup", 7);
					switch (light->GetLightType())
					{
					case LightType::DIRECTIONAL:
						directinalLightCount++;
						mLightingPass.second->Float3(("ldLights[" + std::to_string(directinalLightCount) + "].Direction").c_str(), transform->GetTransform()[2]);
						mLightingPass.second->Float3(("ldLights[" + std::to_string(directinalLightCount) + "].Diffuse").c_str(), light->GetDiffuse() * light->GetIntensity());
						mLightingPass.second->Float3(("ldLights[" + std::to_string(directinalLightCount) + "].Specular").c_str(), light->GetDiffuse() * light->GetIntensity());
						break;
					case LightType::POINT:
						pointLightCount++;
						mLightingPass.second->Float3(("lpLights[" + std::to_string(pointLightCount) + "].Position").c_str(), transform->Position);
						mLightingPass.second->Float3(("lpLights[" + std::to_string(pointLightCount) + "].Diffuse").c_str(), light->GetDiffuse() * light->GetIntensity());
						mLightingPass.second->Float3(("lpLights[" + std::to_string(pointLightCount) + "].Specular").c_str(), light->GetDiffuse() * light->GetIntensity());
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

		objectindex = -1;
		for (auto& object : scene->GetSceneObjects())
		{
			objectindex++;
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

				if (objectindex == mPickedObjectId)
				{
					Drawable* drawable = object->GetProperty<Drawable>();
					if (drawable)
					{
						glEnable(GL_STENCIL_TEST);
						glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
						glStencilMask(0x00);
						glDisable(GL_DEPTH_TEST);
						mOutline->Use();
						for (auto& mesh : drawable->GetMeshes())
						{
							mOutline->Mat4("uProjection", camera->Projection());
							mOutline->Mat4("uViewProjection", camera->ViewProjection());
							Transform* transform = object->GetProperty<Transform>();
							mOutline->Mat4("uTransform", glm::scale(transform->GetTransform(), glm::vec3(1.02f, 1.02f, 1.02f)));
							mesh.first->Bind();

							switch (drawable->GetDrawableType())
							{
							case DrawableType::CUBE:
								glDrawArrays(GL_TRIANGLES, 0, 36);
								break;
							case DrawableType::SPHERE:
							{
								uint32_t scount = mesh.first->GetCount();
								glDrawElements(GL_TRIANGLE_STRIP, scount, GL_UNSIGNED_INT, nullptr);
							}
							break;
							case DrawableType::MODEL:
							{
								uint32_t mcount = mesh.first->GetCount();
								glDrawElements(GL_TRIANGLES, mcount, GL_UNSIGNED_INT, nullptr);
							}
							break;
							}
						}
						glStencilFunc(GL_ALWAYS, 0, 0xFF);
						glStencilMask(0xFF);
					}
				}
			}
		}
		mLightingPass.first->UnBind();

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