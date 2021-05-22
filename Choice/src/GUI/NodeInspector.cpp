#include "NodeInspector.h"

#include <imgui.h>
#include "FontAwesome.h"

#include "Choice.h"

namespace choice
{
	NodeInspector::NodeInspector()
	{
	}

	NodeInspector::~NodeInspector()
	{
	}

	static void TransformUI(const std::string& label, glm::vec3& value, float resetValue)
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
		}
		ImGui::PopStyleColor(3);

		ImGui::TableSetColumnIndex(2);
		ImGui::SetNextItemWidth(53.0f);
		ImGui::DragFloat("##X", &value.x, 0.2f, 0.0f, 0.0f, "%.2f");

		ImGui::TableSetColumnIndex(3);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.8f, 0.1f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.1f, 1.0f, 0.1f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.8f, 0.1f, 1.0f });
		if (ImGui::Button("Y"))
		{
			value.y = resetValue;
		}
		ImGui::PopStyleColor(3);

		ImGui::TableSetColumnIndex(4);
		ImGui::SetNextItemWidth(53.0f);
		ImGui::DragFloat("##Y", &value.y, 0.2f, 0.0f, 0.0f, "%.2f");

		ImGui::TableSetColumnIndex(5);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.1f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.1f, 0.1f, 1.0f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.1f, 0.8f, 1.0f });
		if (ImGui::Button("Z"))
		{
			value.z = resetValue;
		}
		ImGui::PopStyleColor(3);

		ImGui::TableSetColumnIndex(6);
		ImGui::SetNextItemWidth(53.0f);
		ImGui::DragFloat("##Z", &value.z, 0.2f, 0.0f, 0.0f, "%.2f");
		ImGui::PopID();
	}

	static void DrawNodeTransform(Transform* transform)
	{
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow))
		{
			if (ImGui::BeginTable("##Transform", 7, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit))
			{
				ImGui::TableNextRow();
				TransformUI("Position ", transform->Position, 0.0f);
				ImGui::TableNextRow();
				glm::vec3 rotation = glm::degrees(transform->Rotation);
				TransformUI("Rotation ", rotation, 0.0f);
				transform->Rotation = glm::radians(rotation);
				ImGui::TableNextRow();
				TransformUI("Scale    ", transform->Scale, 1.0f);
				ImGui::EndTable();
			}
		}
	}
	
	template<typename T>
	static void DrawNodeDataType(T* data) { static_assert(false); }

	static void TextureUI(const std::string& label, const std::map<std::string, uint32_t>& samplers,
		TextureMap* map, const std::function<void(TextureMap*)>& func)
	{
		std::string name = label;
		if (!map->name.empty())
		{
			name += "-" + map->name;
		}

		if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow))
		{
			if (ImGui::ImageButton(map->texture ? (void*)(uintptr_t)(map->texture->GetId()) : nullptr, { 50.0f, 50.0f }))
			{
				//TODO : Load Texture
			}

			ImGui::SameLine();
			func(map);

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

						ImGui::Combo("##Materials", &currentitem, comboItems.data(), comboItems.size());

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

				ReflectionData& reflectiondata = Choice::Instance()->GetPipeline()->GetReflectionData();

				UniformBuffer* materialBuffer = reflectiondata.UniformBuffers["Material"];
				auto& samplers = reflectiondata.Samplers;

				auto hasmapfunc = [&material, &materialBuffer](const char* name) {

					auto* data = materialBuffer->MemberData<int>(name, material->Data);

					ImGui::Checkbox("##UseMap", (bool*)data);
					ImGui::SameLine();
				};

				for (auto&& [mapType, map] : material->TextureMaps)
				{
					switch (mapType)
					{
					case TEXTURE_MAP_TYPE::ALBEDO:
						TextureUI("Albedo", samplers, map, [&](TextureMap* map) {

							hasmapfunc("Material.HasAlbedoMap");

							auto* data = materialBuffer->MemberData<glm::vec4>("Material.Color", material->Data);
							ImGui::ColorEdit4("##Color", glm::value_ptr(*data), 
								ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoInputs
								| ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoBorder);
						});

						break;
					case TEXTURE_MAP_TYPE::NORMAL:
						TextureUI("Normal", samplers, map, [&](TextureMap* map) {
							hasmapfunc("Material.HasNormalMap");
						});
						break;
					case TEXTURE_MAP_TYPE::ROUGHNESS:
						TextureUI("Roughness", samplers, map, [&](TextureMap* map) {

							hasmapfunc("Material.HasRoughnessMap");

							auto* data = materialBuffer->MemberData<float>("Material.Roughness", material->Data);
							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
							ImGui::SliderFloat("##Roughness", data, 0.0f, 1.0f);
						});

						break;
					case TEXTURE_MAP_TYPE::METALLIC:
						TextureUI("Metallic", samplers, map, [&](TextureMap* map) {

							hasmapfunc("Material.HasMetallicMap");

							auto* data = materialBuffer->MemberData<float>("Material.Metallic", material->Data);
							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
							ImGui::SliderFloat("##Metallic", data, 0.0f, 1.0f);
						});

						break;
					case TEXTURE_MAP_TYPE::AMBIENT_OCCLUSION:
						TextureUI("Ambient Occlusion", samplers, map, [&](TextureMap* map) {
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

			ReflectionData& reflectiondata = Choice::Instance()->GetPipeline()->GetReflectionData();
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

			ImGui::Text(("Name :" + node->Name).c_str());

			DrawNodeTransform(node->NodeTransform);

			switch (node->node_data_type)
			{
			case NODE_DATA_TYPE::MESH: DrawNodeDataType<Mesh>(static_cast<Mesh*>(node)); break;
			case NODE_DATA_TYPE::LIGHT: DrawNodeDataType<Light>(static_cast<Light*>(node)); break;
			}

			ImGui::End();
		}
	}
}