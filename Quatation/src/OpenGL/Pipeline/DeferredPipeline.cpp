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
		mShadowMapCapturePass.first = new ShadowMapCapture(w, h);
		mShadowMapCapturePass.second = new Shader("Choice/assets/shaders/ShadowMap.glsl");

		mGeometryPass.first = new DeferredGeometryCapture(w, h);
		mGeometryPass.second = new Shader("Choice/assets/shaders/DeferredGeometryPass.glsl");

		mOutline = new Shader("Choice/assets/shaders/Outline.glsl");

		mLightingPass.first = new DeferredLightingCapture(w, h);
		mLightingPass.second = new Shader("Choice/assets/shaders/DeferredLightingPass.glsl");
	}

	void DeferredPipeline::Visible(uint32_t w, uint32_t h)
	{
		mShadowMapCapturePass.first->Visible(w, h);
		mGeometryPass.first->Visible(w, h);
		mLightingPass.first->Visible(w, h);
	}

	void DeferredPipeline::Update(Scene* scene, Camera* camera)
	{
		glEnable(GL_DEPTH_TEST);

		glCullFace(GL_FRONT);
		mShadowMapCapturePass.first->Bind();
		glClear(GL_DEPTH_BUFFER_BIT);

		mShadowMapCapturePass.second->Use();
		for (auto& object : scene->GetSceneObjects())
		{
			if (object)
			{
				Light* light = object->GetProperty<Light>();
				if (light)
				{
					switch (light->GetLightType())
					{
					case LightType::DIRECTIONAL:
						for (auto& _object : scene->GetSceneObjects())
						{
							if (_object)
							{
								Drawable* drawable = _object->GetProperty<Drawable>();
								if (drawable)
								{
									for (auto& mesh : drawable->GetMeshes())
									{
										mShadowMapCapturePass.second->Mat4("uLightViewProjection", light->ViewProjection(_object->GetProperty<Transform>())[0]);
										mShadowMapCapturePass.second->Mat4("uTransform", _object->GetProperty<Transform>()->GetTransform());
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
						break;
					}
				}
			}
		}

		mShadowMapCapturePass.first->UnBind();
		glCullFace(GL_BACK);

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

						//Bind Roughness Map
						if (drawable->GetMaterials()[mesh.second]->RoughnessMap.first
							&& drawable->GetMaterials()[mesh.second]->RoughnessMap.second.first)
						{
							drawable->GetMaterials()[mesh.second]->RoughnessMap.second.first->Bind(2);
							mGeometryPass.second->Int("gHasRoughnessMap", 1);
						}
						else
						{
							mGeometryPass.second->Int("gHasRoughnessMap", 0);
						}

						//Bind Metallic Map
						if (drawable->GetMaterials()[mesh.second]->MetallicMap.first
							&& drawable->GetMaterials()[mesh.second]->MetallicMap.second.first)
						{
							drawable->GetMaterials()[mesh.second]->MetallicMap.second.first->Bind(3);
							mGeometryPass.second->Int("gHasMetallicMap", 1);
						}
						else
						{
							mGeometryPass.second->Int("gHasMetallicMap", 0);
						}

						//Bind Ambient Occlusion Map
						if (drawable->GetMaterials()[mesh.second]->AOMap.first
							&& drawable->GetMaterials()[mesh.second]->AOMap.second.first)
						{
							drawable->GetMaterials()[mesh.second]->AOMap.second.first->Bind(4);
							mGeometryPass.second->Int("gHasAmbientOcclusionMap", 1);
						}
						else
						{
							mGeometryPass.second->Int("gHasAmbientOcclusionMap", 0);
						}

						mGeometryPass.second->Int("gMaterial.Diffuse", 0);
						mGeometryPass.second->Int("gMaterial.Normal", 1);
						mGeometryPass.second->Int("gMaterial.Roughness", 2);
						mGeometryPass.second->Int("gMaterial.Metallic", 3);
						mGeometryPass.second->Int("gMaterial.AmbientOcclusion", 4);
						mGeometryPass.second->Float("gMaterial.RoughnessFactor", drawable->GetMaterials()[mesh.second]->Roughness);
						mGeometryPass.second->Float("gMaterial.MetallicFactor", drawable->GetMaterials()[mesh.second]->Metallic);
						mGeometryPass.second->Float("gMaterial.AO", drawable->GetMaterials()[mesh.second]->Ao);
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

					//Calculating Scene AABB
					glm::vec4 min = object->GetProperty<Transform>()->GetTransform() * 
						glm::vec4(drawable->GetBoundingBox().second.Min, 1.0f);
					
					scene->GetBoundingBox().second.Min.x = min.x < scene->GetBoundingBox().second.Min.x ? min.x : scene->GetBoundingBox().second.Min.x;
					scene->GetBoundingBox().second.Min.y = min.y < scene->GetBoundingBox().second.Min.y ? min.y : scene->GetBoundingBox().second.Min.y;
					scene->GetBoundingBox().second.Min.z = min.z < scene->GetBoundingBox().second.Min.z ? min.z : scene->GetBoundingBox().second.Min.z;

					glm::vec4 max = object->GetProperty<Transform>()->GetTransform() *
						glm::vec4(drawable->GetBoundingBox().second.Max, 1.0f);	

					scene->GetBoundingBox().second.Max.x = max.x > scene->GetBoundingBox().second.Max.x ? max.x : scene->GetBoundingBox().second.Max.x;
					scene->GetBoundingBox().second.Max.y = max.y > scene->GetBoundingBox().second.Max.y ? max.y : scene->GetBoundingBox().second.Max.y;
					scene->GetBoundingBox().second.Max.z = max.z > scene->GetBoundingBox().second.Max.z ? max.z : scene->GetBoundingBox().second.Max.z;

					mGeometryPass.second->Mat4("uViewProjection", camera->ViewProjection());
					mGeometryPass.second->Mat4("uTransform", object->GetProperty<Transform>()->GetTransform());

					drawable->GetBoundingBox().first->Bind();
					uint32_t count = (uint32_t)drawable->GetBoundingBox().first->GetCount();
					glDrawElements(GL_LINES, count, GL_UNSIGNED_INT, 0);
					glLineWidth(2.0f);
				}
			}
		}

		//Drawing Scene AABB
		scene->GetBoundingBox().first = CreateBoundingBox(ExpandAABB(scene->GetBoundingBox().second));
		mGeometryPass.second->Use();
		mGeometryPass.second->Mat4("uViewProjection", camera->ViewProjection());
		mGeometryPass.second->Mat4("uTransform", glm::mat4(1.0f));
		scene->GetBoundingBox().first->Bind();
		uint32_t count = (uint32_t)scene->GetBoundingBox().first->GetCount();
		glDrawElements(GL_LINES, count, GL_UNSIGNED_INT, 0);
		glLineWidth(2.0f);
		delete scene->GetBoundingBox().first;

		//Calculate Scene AABB In Light Space
		BoundingBox lightAABB = CalculateBoundingBox(nullptr, 0, 0);
		auto expanded = ExpandAABB(scene->GetBoundingBox().second);

		glm::mat4 lightview = scene->GetSceneObjects()[1]->GetProperty<Light>()->View(scene->GetSceneObjects()[1]->GetProperty<Transform>());

		for (auto& point : expanded)
		{
			const glm::vec4 pointinview = lightview * glm::vec4(point, 1.0f);
			
			lightAABB.Min.x = pointinview.x < lightAABB.Min.x ? pointinview.x : lightAABB.Min.x;
			lightAABB.Min.y = pointinview.y < lightAABB.Min.y ? pointinview.y : lightAABB.Min.y;
			lightAABB.Min.z = pointinview.z < lightAABB.Min.z ? pointinview.z : lightAABB.Min.z;

			lightAABB.Max.x = pointinview.x > lightAABB.Max.x ? pointinview.x : lightAABB.Max.x;
			lightAABB.Max.y = pointinview.y > lightAABB.Max.y ? pointinview.y : lightAABB.Max.y;
			lightAABB.Max.z = pointinview.z > lightAABB.Max.z ? pointinview.z : lightAABB.Max.z;
		}

		//Draw LightSpace AABB
		expanded.clear();
		expanded = ExpandAABB(lightAABB);
		for (auto& point : expanded)
		{
			glm::vec4 temp = glm::inverse(lightview) * glm::vec4(point, 1.0f);
			point.x = temp.x;
			point.y = temp.y;
			point.z = temp.z;
		}

		VertexArray* va = CreateBoundingBox(expanded);
		va->Bind();
		uint32_t count_ = va->GetCount();
		glDrawElements(GL_LINES, count_, GL_UNSIGNED_INT, nullptr);
		glLineWidth(2.0f);
		delete va;

		scene->GetBoundingBox().second = CalculateBoundingBox(nullptr, 0, 0);

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
					mShadowMapCapturePass.first->BindCaptures({ 8, 9 });
					mLightingPass.second->Int("lGBuffer.Position", 0);
					mLightingPass.second->Int("lGBuffer.Normal", 1);
					mLightingPass.second->Int("lGBuffer.AlbedoS", 2);
					mLightingPass.second->Int("lGBuffer.RoughMetalAo", 3);
					mLightingPass.second->Int("lIBL.IrradianceMap", 5);
					mLightingPass.second->Int("lIBL.PreFilterMap", 6);
					mLightingPass.second->Int("lIBL.BRDFLookup", 7);
					mLightingPass.second->Int("lFragposLightSpace", 8);
					mLightingPass.second->Int("lShadowMap", 9);
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
		delete mShadowMapCapturePass.first;
		delete mShadowMapCapturePass.second;
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

	ShadowMapCapture::ShadowMapCapture(uint32_t w, uint32_t h)
		:Framebuffer(w, h)
	{
		Invalidate();
	}

	ShadowMapCapture::~ShadowMapCapture()
	{
		Framebuffer::Destroy();
		glDeleteTextures(1, &mShadowmap);
		glDeleteTextures(1, &mFragpos);
	}

	void ShadowMapCapture::BindCaptures(glm::uvec2 slots) const
	{
		glActiveTexture(GL_TEXTURE0 + slots.x);
		glBindTexture(GL_TEXTURE_2D, mFragpos);
		glActiveTexture(GL_TEXTURE0 + slots.y);
		glBindTexture(GL_TEXTURE_2D, mShadowmap);
	}

	void ShadowMapCapture::Invalidate()
	{
		if (mRendererId)
		{
			Framebuffer::Destroy();
			glDeleteTextures(1, &mShadowmap);
			glDeleteTextures(1, &mFragpos);
		}

		glCreateFramebuffers(1, &mRendererId);
		glBindFramebuffer(GL_FRAMEBUFFER, mRendererId);

		glCreateTextures(GL_TEXTURE_2D, 1, &mFragpos);
		glBindTexture(GL_TEXTURE_2D, mFragpos);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, mWidth, mHeight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mFragpos, 0);

		glCreateTextures(GL_TEXTURE_2D, 1, &mShadowmap);
		glBindTexture(GL_TEXTURE_2D, mShadowmap);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, mWidth, mHeight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mShadowmap, 0);

		glDrawBuffer(GL_COLOR_ATTACHMENT0);

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