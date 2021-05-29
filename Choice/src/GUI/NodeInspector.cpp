#include "NodeInspector.h"

#include <imgui.h>
#include "FontAwesome.h"

#include "ReflectionData.h"
#include "Input.h"
#include "FileDialog.h"

#include "Choice.h"

namespace choice
{
	NodeInspector::NodeInspector()
	{
	}

	NodeInspector::~NodeInspector()
	{
	}

	static void TransformUI(const std::string& label, glm::vec3& value, float resetValue, bool* ischanged)
	{
		ImGui::PushID(label.c_str());
		ImGui::TableSetColumnIndex(0);
		ImGui::Text(label.c_str());

		ImGui::TableSetColumnIndex(1);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.1f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 1.0f, 0.1f, 0.1f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.1f, 1.0f });
		if (ImGui::Button("X"))
		{
			value.x = resetValue;
			*ischanged = true;
		}
		ImGui::PopStyleColor(3);

		ImGui::TableSetColumnIndex(2);
		ImGui::SetNextItemWidth(53.0f);
		if (ImGui::DragFloat("##X", &value.x, 0.2f, 0.0f, 0.0f, "%.2f"))
		{
			*ischanged = true;
		}

		ImGui::TableSetColumnIndex(3);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.8f, 0.1f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.1f, 1.0f, 0.1f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.8f, 0.1f, 1.0f });
		if (ImGui::Button("Y"))
		{
			value.y = resetValue;
			*ischanged = true;
		}
		ImGui::PopStyleColor(3);

		ImGui::TableSetColumnIndex(4);
		ImGui::SetNextItemWidth(53.0f);
		if (ImGui::DragFloat("##Y", &value.y, 0.2f, 0.0f, 0.0f, "%.2f"))
		{
			*ischanged = true;
		}

		ImGui::TableSetColumnIndex(5);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.1f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.1f, 0.1f, 1.0f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.1f, 0.8f, 1.0f });
		if (ImGui::Button("Z"))
		{
			value.z = resetValue;
			*ischanged = true;
		}
		ImGui::PopStyleColor(3);

		ImGui::TableSetColumnIndex(6);
		ImGui::SetNextItemWidth(53.0f);
		if (ImGui::DragFloat("##Z", &value.z, 0.2f, 0.0f, 0.0f, "%.2f"))
		{
			*ischanged = true;
		}
		ImGui::PopID();
	}

	static bool DrawNodeTransform(Transform* transform) //Returns True If Something Changed In TransformUI
	{
		bool ischanged = false;
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow))
		{
			if (ImGui::BeginTable("##Transform", 7, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit))
			{
				ImGui::TableNextRow();
				TransformUI("Position ", transform->Position, 0.0f, &ischanged);
				ImGui::TableNextRow();
				glm::vec3 rotation = glm::degrees(transform->Rotation);
				TransformUI("Rotation ", rotation, 0.0f, &ischanged);
				transform->Rotation = glm::radians(rotation);
				ImGui::TableNextRow();
				TransformUI("Scale    ", transform->Scale, 1.0f, &ischanged);
				ImGui::EndTable();
			}
		}
		return ischanged;
	}
	
	template<typename T>
	static void DrawNodeDataType(T* data) { static_assert(false); }

	static void TextureUI(const std::string& label, TEXTURE_MAP_TYPE type,
		TextureMap* map, const std::function<void(void)>& func)
	{
		std::string name = label;
		if (!map->filepath.empty())
		{
			name += "-" + ghc::filesystem::path(map->filepath).stem().string();
		}

		if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow))
		{
			if (ImGui::ImageButton(map->texture ? (void*)(uintptr_t)(map->texture->GetId()) : nullptr, { 50.0f, 50.0f }))
			{
				std::string imagefilapath = FileDialog::OpenFile("All Files"); //TODO : Change All Files To Specific Format
				if (!imagefilapath.empty())
				{
					BlockCompressionFormat format;

					switch (type)
					{
					case TEXTURE_MAP_TYPE::ALBEDO: format = BlockCompressionFormat::BC1; break;
					case TEXTURE_MAP_TYPE::NORMAL: format = BlockCompressionFormat::BC5; break;
					case TEXTURE_MAP_TYPE::ROUGHNESS: 
					case TEXTURE_MAP_TYPE::METALLIC:
					case TEXTURE_MAP_TYPE::AMBIENT_OCCLUSION:
						format = BlockCompressionFormat::BC4; 
						break;
					}

					map->filepath = CompressTexture(imagefilapath, format, true);
					if (map->texture) { delete map->texture; }
					map->texture = new Texture2D(LoadTexture2D(map->filepath));
				}
			}

			ImGui::SameLine();
			func();

			ImGui::TreePop();
		}
	}

	template<>
	static void DrawNodeDataType<Mesh>(Mesh* mesh)
	{
		if (ImGui::CollapsingHeader(ICON_FK_CUBE "Mesh", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow))
		{
			ImGui::Text("Materials Interface ");
			ImGui::SameLine();
			static bool materialsInterface = true;
			ImGui::Checkbox("##MaterialsInterface", &materialsInterface);

			if (materialsInterface)
			{
				ImGui::Separator();

				std::vector<const char*> comboItems;
				if (comboItems.size() != mesh->primitives.size())
				{
					for (auto& primitve : mesh->primitives)
					{
						comboItems.push_back(primitve->material->Name.c_str());
					}
				}

				static int currentitem = 0; //will be set In Combo

				static bool showAddMapMenu = false;

				if (ImGui::BeginTable("##Materials", 3, ImGuiTableFlags_SizingFixedFit))
				{
					for (int row = 0; row < 1; row++)
					{
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Materials ");

						ImGui::TableSetColumnIndex(1);
						ImGui::SetNextItemWidth(225.0f);
						ImGui::Combo("##Materials", &currentitem, comboItems.data(), static_cast<int>(comboItems.size()));

						ImGui::TableSetColumnIndex(2);
						if (ImGui::Button("+"))
						{
							showAddMapMenu = true;
						}
					}
					ImGui::EndTable();
				}

				Material* material = mesh->primitives[currentitem]->material;

				auto mapexistfunc = [&material](TEXTURE_MAP_TYPE map_type) {
					if (material->TextureMaps.find(map_type) != material->TextureMaps.end())
						return true;
					return false;
				};

				if (showAddMapMenu)
				{
					ImGui::OpenPopup("AddTextureMap");

					if (ImGui::BeginPopup("AddTextureMap"))
					{
						if (!ImGui::IsWindowHovered()) //Close Pop-up If Clicked Out Of It
						{
							if (Input::IsButtonPressed(Mouse::BUTTON1)) { showAddMapMenu = false; }
						}

						if (ImGui::MenuItem("Albedo"))
						{
							if (!mapexistfunc(TEXTURE_MAP_TYPE::ALBEDO))
							{
								material->TextureMaps.insert({ TEXTURE_MAP_TYPE::ALBEDO, new TextureMap() });
							}
							showAddMapMenu = false;
						}

						if (ImGui::MenuItem("Normal"))
						{
							if (!mapexistfunc(TEXTURE_MAP_TYPE::NORMAL))
							{
								material->TextureMaps.insert({ TEXTURE_MAP_TYPE::NORMAL, new TextureMap() });
							}
							showAddMapMenu = false;
						}

						if (ImGui::MenuItem("Roughness"))
						{
							if (!mapexistfunc(TEXTURE_MAP_TYPE::ROUGHNESS))
							{
								material->TextureMaps.insert({ TEXTURE_MAP_TYPE::ROUGHNESS, new TextureMap() });
							}
							showAddMapMenu = false;
						}

						if (ImGui::MenuItem("Metallic"))
						{
							if (!mapexistfunc(TEXTURE_MAP_TYPE::METALLIC))
							{
								material->TextureMaps.insert({ TEXTURE_MAP_TYPE::METALLIC, new TextureMap() });
							}
							showAddMapMenu = false;
						}

						if (ImGui::MenuItem("Ambient Occlusion"))
						{
							if (!mapexistfunc(TEXTURE_MAP_TYPE::AMBIENT_OCCLUSION))
							{
								material->TextureMaps.insert({ TEXTURE_MAP_TYPE::AMBIENT_OCCLUSION, new TextureMap() });
							}
							showAddMapMenu = false;
						}

						ImGui::EndPopup();
					}
				}

				ReflectionData& reflectiondata = global::GlobalReflectionData;

				UniformBuffer* materialBuffer = reflectiondata.UniformBuffers["Material"];

				auto hasmapfunc = [&material, &materialBuffer](const char* name) {

					auto* data = materialBuffer->MemberData<int>(name, material->Data);

					ImGui::Checkbox("##UseMap", (bool*)data);
				};

				//Color
				ImGui::Text("Color ");
				ImGui::SameLine();
				auto* colordata = materialBuffer->MemberData<glm::vec3>("Material.Color", material->Data);
				ImGui::ColorEdit3("##Color", glm::value_ptr(*colordata),
					ImGuiColorEditFlags_PickerHueWheel
					| ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoBorder);

				//Roughness Factor
				ImGui::Text("Roughness ");
				ImGui::SameLine();
				auto* roughnessdata = materialBuffer->MemberData<float>("Material.Roughness", material->Data);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
				ImGui::SliderFloat("##Roughness", roughnessdata, 0.0f, 1.0f);

				//Metallic Factor
				ImGui::Text("Metallic       ");
				ImGui::SameLine();
				auto* metallicdata = materialBuffer->MemberData<float>("Material.Metallic", material->Data);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
				ImGui::SliderFloat("##Metallic", metallicdata, 0.0f, 1.0f);

				ImGui::Separator();

				for (auto&& [mapType, map] : material->TextureMaps)
				{
					switch (mapType)
					{
					case TEXTURE_MAP_TYPE::ALBEDO:
						TextureUI("Albedo", mapType, map, [&]() {
							hasmapfunc("Material.HasAlbedoMap");
						});
						break;
					case TEXTURE_MAP_TYPE::NORMAL:
						TextureUI("Normal", mapType, map, [&]() {
							hasmapfunc("Material.HasNormalMap");
						});
						break;
					case TEXTURE_MAP_TYPE::ROUGHNESS:
						TextureUI("Roughness", mapType, map, [&]() {
							hasmapfunc("Material.HasRoughnessMap");
						});
						break;
					case TEXTURE_MAP_TYPE::METALLIC:
						TextureUI("Metallic", mapType, map, [&]() {
							hasmapfunc("Material.HasMetallicMap");
						});
						break;
					case TEXTURE_MAP_TYPE::AMBIENT_OCCLUSION:
						TextureUI("Ambient Occlusion", mapType, map, [&]() {
							hasmapfunc("Material.HasAoMap");
						});
						break;
					}
				}
			}
		}
	}

	template<>
	static void DrawNodeDataType<Light>(Light* light)
	{
		if (ImGui::CollapsingHeader(ICON_FK_LIGHTBULB_O "Light", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow))
		{
			ImGui::Text("Light Type ");
			ImGui::SameLine();
			const char* comboItems[2] = { "Directional", "Point" };
			static int currentItem = static_cast<int>(light->Type);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
			ImGui::Combo("##LightType", &currentItem, comboItems, IM_ARRAYSIZE(comboItems));

			ReflectionData& reflectiondata = global::GlobalReflectionData;
			auto* lightBuffer = reflectiondata.UniformBuffers["Lights"];

			auto colorfunc = [&light, &lightBuffer](const char* name) {

				ImGui::Text("Color ");
				ImGui::SameLine();

				auto* data = lightBuffer->MemberData<glm::vec3>(name, light->Data);
				ImGui::ColorEdit3("##LightColor", glm::value_ptr(*data), ImGuiColorEditFlags_PickerHueWheel
					| ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoBorder);
			};

			switch (light->Type)
			{
			case LIGHT_TYPE::DIRECTIONAL:
				colorfunc("Lights.ldLights.Color");
				break;
			case LIGHT_TYPE::POINT:
				colorfunc("Lights.lpLights.Color");

				auto* radiusdata = lightBuffer->MemberData<float>("Lights.lpLights.Radius", light->Data);

				ImGui::Text("Radius ");
				ImGui::SameLine();
				ImGui::SliderFloat("##Radius", radiusdata, 0.0f, 100.0f);
				break;
			}
		}
	}

	void NodeInspector::Execute(Node* node)
	{
		if (node)
		{
			ImGui::Begin(ICON_FK_INFO_CIRCLE" Inspector");

			if (ImGui::IsWindowFocused() || ImGui::IsWindowHovered())
			{
				Choice::Instance()->GetEditor()->GetCamera()->AcceptInput(false);
			}

			ImGui::Text(("Name :" + node->Name).c_str());

			if (DrawNodeTransform(node->NodeTransform) && node->node_data_type == NODE_DATA_TYPE::NONE)
			{
				UpdateWorldTransform(node);
			}

			switch (node->node_data_type)
			{
			case NODE_DATA_TYPE::MESH: DrawNodeDataType<Mesh>(static_cast<Mesh*>(node)); break;
			case NODE_DATA_TYPE::LIGHT: DrawNodeDataType<Light>(static_cast<Light*>(node)); break;
			}

			ImGui::End();
		}
	}
}