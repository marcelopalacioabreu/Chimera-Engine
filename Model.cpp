#include "Model.h"
#include "Application.h"
#include "ModuleCamera.h"
#include "ModuleTextures.h"
#include "ModuleProgram.h"
#include "ModuleInput.h"
#include "imgui.h"

Model::Model(CustomFile& modelFile) {
	LoadModel(modelFile);
	App->camera->selectedObject = this;
}

Model::~Model() { }

bool Model::LoadModel(CustomFile& file) {

	const aiScene* scene = aiImportFile(file.path, { aiProcess_Triangulate | aiProcess_GenUVCoords });

	if (scene) {
		GenerateMeshData(scene->mRootNode, scene);
		GenerateMaterialData(scene);
		GetAABB();
	} else {
		LOG("Error: %s", aiGetErrorString());
	}

	return scene;
}

void Model::GenerateMeshData(const aiNode* node, const aiScene* scene) {
	assert(scene != nullptr);

	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.emplace_back(mesh);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		GenerateMeshData(node->mChildren[i], scene);
	}

}

void Model::Draw() const {

	for (auto &mesh : meshes) {
		mesh.Draw(App->program->textureProgram, textures);
	}

}

void Model::DrawInfo() const {

	// TODO: this is weird, change collapsing to other imgui element
	if (ImGui::CollapsingHeader("Meshes loaded")) {

		for (auto& meshSelected : meshes) {

			if (ImGui::CollapsingHeader(meshSelected.name, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick)) {

				if (&meshSelected != nullptr) {

					ImGui::Text("Triangles Count: %d", meshSelected.numIndices / 3);
					ImGui::Text("Vertices Count: %d", meshSelected.vertices.size());
					ImGui::Text("Mesh size:\n X: %f | Y: %f | Z: %f", meshSelected.bbox.Size().x, meshSelected.bbox.Size().y, meshSelected.bbox.Size().z);

				} else {
					ImGui::Text("No mesh attached");
				}

			}
		}
	}

	if (ImGui::CollapsingHeader("Texture")) {

		for (auto &texture : textures) {
			ImGui::Text("Size:  Width: %d | Height: %d", texture.width, texture.height);
			float size = ImGui::GetWindowWidth();
			ImGui::Image((ImTextureID)texture.id, { size,size });
		}
	}

}


void Model::UpdateTexture(Texture texture) {

	for (auto &Oldtexture : textures) {
		Oldtexture = texture;
	}

}

void Model::GenerateMaterialData(const aiScene* scene) {
	assert(scene != nullptr);

	for (unsigned i = 0; i < scene->mNumMaterials; ++i) {

		const aiMaterial* materialSrc = scene->mMaterials[i];

		aiString file;
		aiTextureMapping mapping = aiTextureMapping_UV;

		if (materialSrc->GetTexture(aiTextureType_DIFFUSE, 0, &file, &mapping, 0) == AI_SUCCESS) {
			std::string newTexturePath(modelFile.path);
			newTexturePath += file.C_Str();
			CustomFile& newTexture = CustomFile(newTexturePath.c_str());
			App->input->files.emplace_back(newTexture);
			textures.push_back(App->textures->Load(newTexture));
		} else {
			LOG("Error: Could not load the %fth material", i);
		}

	}

}

// Axis Aligned Bounding Box
void Model::GetAABB() {

	for (auto& mesh : meshes) {
		boundingBox.Enclose(mesh.bbox);
	}

}
