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
		uint32_t tex[4] = { mPositionId, mNormalId, mAlbedoSId, mDepthStencilId };
		glDeleteTextures(4, tex);
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

	void DeferredGeometryCapture::Invalidate()
	{
		if (mRendererId)
		{
			Framebuffer::Destroy();
			uint32_t tex[4] = { mPositionId, mNormalId, mAlbedoSId, mDepthStencilId };
			glDeleteTextures(4, tex);
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
		mMousePickingPass.first = new MousePickingCapture(w, h);
		mMousePickingPass.second = new Shader("Choice/assets/shaders/MousePickingPass.glsl");

		mOutline = new Shader("Choice/assets/shaders/Outline.glsl");

		mGeometryPass.first = new DeferredGeometryCapture(w, h);
		mGeometryPass.second = new Shader("Choice/assets/shaders/DeferredGeometryPass.glsl");

		mLightingPass = new Shader("Choice/assets/shaders/DeferredLightingPass.glsl");
	}

	void DeferredPipeline::Visible(uint32_t w, uint32_t h)
	{
		mMousePickingPass.first->Visible(w, h);
		mGeometryPass.first->Visible(w, h);
	}

	void DeferredPipeline::Update(Scene* scene, Camera* camera)
	{
		glEnable(GL_DEPTH_TEST);

		mMousePickingPass.first->Bind();
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
					int drawindex = -1;
					for (auto& mesh : model->Meshes)
					{
						drawindex++;
						mMousePickingPass.second->Use();
						mMousePickingPass.second->UInt("pObjectIndex", objectindex);
						mMousePickingPass.second->UInt("pDrawIndex", drawindex);
						mMousePickingPass.second->Mat4("uViewProjection", camera->ViewProjection());
						mMousePickingPass.second->Mat4("uTransform", object->GetProperty<Transform>()->GetTransform());
						mesh.first->Bind();
						uint32_t count = mesh.first->GetCount();
						glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
					}
				}
			}
		}
		mMousePickingPass.first->UnBind();

		if (Input::IsButtonPressed(Mouse::BUTTON1) && 
			!(Input::IsKeyPressed(Key::LEFTSHIFT) || Input::IsKeyPressed(Key::LEFTALT)) 
			&& mMousePicking)
		{
			PixelInfo* pixelinfo = mMousePickingPass.first->ReadPixels((uint32_t)Input::GetMouseX(),
				Choice::Instance()->GetWindow()->GetHeight() - (uint32_t)Input::GetMouseY() - 1);
			if (pixelinfo->PrimitiveId) { mPickedObjectId = static_cast<int>(pixelinfo->ObjectId); }
			else { mPickedObjectId = -1; }
			Choice::Instance()->GetEditor()->SetSelectedObjectIndex(mPickedObjectId);
		}

		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		mGeometryPass.first->Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		int _objectindex = -1;
		for (auto& object : scene->GetSceneObjects())
		{
			_objectindex++;
			if (object)
			{
				Skybox* skybox = object->GetProperty<Skybox>();
				if (skybox)
				{
					glDepthFunc(GL_LEQUAL);
					skybox->Draw(camera);
					glDepthFunc(GL_LESS);
				}

				Model* model = object->GetProperty<Model>();
				if (model)
				{
					if (_objectindex == mPickedObjectId)
					{				
						glClear(GL_STENCIL_BUFFER_BIT);
						glStencilFunc(GL_ALWAYS, 1, 0xFF);
						glStencilMask(0xFF);
					}
					else
					{
						glClear(GL_STENCIL_BUFFER_BIT);
						glStencilMask(0x00);
					}
					for (auto& mesh : model->Meshes)
					{
						mGeometryPass.second->Use();
						if (model->Materials[mesh.second]->DiffuseMap) 
						{ model->Materials[mesh.second]->DiffuseMap->Bind(0); }
						if (model->Materials[mesh.second]->NormalMap) 
						{ model->Materials[mesh.second]->NormalMap->Bind(1); }
						mGeometryPass.second->Int("gMaterial.Diffuse", 0);
						mGeometryPass.second->Int("gMaterial.Normal", 1);
						mGeometryPass.second->Mat4("uViewProjection", camera->ViewProjection());
						mGeometryPass.second->Mat4("uTransform", object->GetProperty<Transform>()->GetTransform());
						mesh.first->Bind();
						uint32_t count = mesh.first->GetCount();
						glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
					}

					if (_objectindex == mPickedObjectId)
					{
						glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
						glStencilMask(0x00);
						glDisable(GL_DEPTH_TEST);
						for (auto& mesh : model->Meshes)
						{
							mOutline->Use();
							mOutline->Mat4("uViewProjection", camera->ViewProjection());
							Transform* transform = scene->GetSceneObjects()[mPickedObjectId]->GetProperty<Transform>();
							mOutline->Mat4("uTransform", glm::scale(transform->GetTransform(), glm::vec3(1.02f, 1.02f, 1.02f)));
							mesh.first->Bind();
							uint32_t count = mesh.first->GetCount();
							glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
						}
						glStencilMask(0xFF);
						glStencilFunc(GL_ALWAYS, 0, 0xFF);
						glEnable(GL_DEPTH_TEST);
					}
				}

			}
		}
		mGeometryPass.first->UnBind();

		glDisable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT);
		mLightingPass->Use();
		mGeometryPass.first->BindGBuffer({ 0, 1, 2, 3 });
		mLightingPass->Int("gBuffer.AlbedoS", 2);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	void DeferredPipeline::Shutdown()
	{
		delete mMousePickingPass.first;
		delete mMousePickingPass.second;
		delete mOutline;
		delete mGeometryPass.first;
		delete mGeometryPass.second;
		delete mLightingPass;
	}

	MousePickingCapture::MousePickingCapture(uint32_t w, uint32_t h)
		:Framebuffer(w, h)
	{
		mPixelInfo = new PixelInfo();
		Invalidate();
	}

	MousePickingCapture::~MousePickingCapture()
	{
		Framebuffer::Destroy();
		glDeleteTextures(1, &mPickingId);
		delete mPixelInfo;
	}

	PixelInfo* MousePickingCapture::ReadPixels(uint32_t xpos, uint32_t ypos)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mRendererId);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glReadPixels(static_cast<GLint>(xpos), static_cast<GLint>(ypos), 1, 1, GL_RGB, GL_FLOAT, mPixelInfo);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return mPixelInfo;
	}

	void MousePickingCapture::Invalidate()
	{
		if (mRendererId)
		{
			Framebuffer::Destroy();
			glDeleteTextures(1, &mPickingId);
		}

		glCreateFramebuffers(1, &mRendererId);
		glBindFramebuffer(GL_FRAMEBUFFER, mRendererId);

		glCreateTextures(GL_TEXTURE_2D, 1, &mPickingId);
		glBindTexture(GL_TEXTURE_2D, mPickingId);

		glCreateTextures(GL_TEXTURE_2D, 1, &mPickingId);
		glBindTexture(GL_TEXTURE_2D, mPickingId);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, mWidth, mHeight);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mPickingId, 0);

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