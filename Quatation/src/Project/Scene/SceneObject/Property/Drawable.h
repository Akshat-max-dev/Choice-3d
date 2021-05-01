#pragma once
#include "cpch.h"

#include "OpenGL/VertexArray.h"
#include "OpenGL/Material.h"
#include "BoundingBox.h"

namespace choice
{
	enum class DrawableType
	{
		NONE = -1, MODEL = 0, CUBE = 1, SPHERE = 2, LINE = 3
	};

	class Drawable
	{
	public:
		Drawable(const std::string& name, DrawableType type);
		~Drawable();

		std::vector<Material*>& GetMaterials() { return mMaterials; }
		std::vector<std::pair<VertexArray*, uint32_t>>& GetMeshes() { return mMeshes; }
		std::pair<VertexArray*, BoundingBox>& GetBoundingBox() { return mBoundingBox; }
		std::string& GetName() { return mName; }
		DrawableType& GetDrawableType() { return mDrawableType; }
	private:
		std::vector<Material*> mMaterials;
		std::pair<VertexArray*, BoundingBox> mBoundingBox;
		std::vector<std::pair<VertexArray*, uint32_t>> mMeshes;
		std::string mName;
		DrawableType mDrawableType = DrawableType::NONE;
	};

	const std::string DumpDrawable(const std::string& srcFile, const std::string& dstDirectory, DrawableType type);
	Drawable* LoadDrawable(const std::string& src, DrawableType type, bool loadMaterials);
}