#pragma once
#include "cpch.h"

#include "OpenGL/VertexArray.h"
#include "OpenGL/Material.h"
#include "BoundingBox.h"
#include "Animation/Animation.h"

namespace choice
{
	class Animation;

	enum class DrawableType
	{
		NONE = -1, MODEL = 0, CUBE = 1, SPHERE = 2
	};

	class Drawable
	{
	public:
		Drawable(const std::string& name, DrawableType type);
		~Drawable();

		std::vector<Material*>& GetMaterials() { return mMaterials; }
		std::vector<std::pair<VertexArray*, uint32_t>>& GetMeshes() { return mMeshes; }

		BoundingBox& GetBoundingBox() { return mBoundingBox; }
		std::string& GetName() { return mName; }
		DrawableType& GetDrawableType() { return mDrawableType; }

		Animation* GetAnimation() { return mAnimation; }
		void SetAnimation(Animation* animation) { mAnimation = animation; }
	private:
		std::string mName;
		DrawableType mDrawableType = DrawableType::NONE;

		BoundingBox mBoundingBox;
		std::vector<Material*> mMaterials;
		std::vector<std::pair<VertexArray*, uint32_t>> mMeshes;

		Animation* mAnimation;
	};

	const std::string DumpDrawable(const std::string& srcFile, const std::string& dstDirectory, DrawableType type);
	Drawable* LoadDrawable(const std::string& src, DrawableType type, bool loadMaterials);
}