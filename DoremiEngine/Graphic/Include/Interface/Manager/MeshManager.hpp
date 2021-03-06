#pragma once
#include <string>
#include <vector>
#include <DoremiEditor/Core/Include/MaterialMessage.hpp>
#include <DoremiEditor/Core/Include/MaterialData.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Texture/SpriteInfo.hpp>

namespace DirectX
{
    struct XMFLOAT4X4;
    struct XMFLOAT2;
}
namespace DoremiEngine
{
    namespace Graphic
    {
        struct Vertex;
        struct SkeletalVertex;
        class MeshInfo;
        class SpriteInfo;
        class MaterialInfo;

        class MeshManager
        {
        public:
            // Builds a mesh from the given file, the filename then becomes the mesh's id
            virtual MeshInfo* BuildMeshInfo(const std::string& p_fileName) = 0;
            virtual MeshInfo* BuildQuadMeshInfo(const std::string& p_fileName) = 0;
            // Builds a mesh from the latlines and longlines TODOLH figure out how the latlines and longlines works and improve this comment
            virtual MeshInfo* BuildSphereMeshInfo(const std::string& p_fileName, int p_latLines, int p_longLines) = 0;
            // Builds a mesh from the given vector of vertices, the given string becomes the mesh's id
            virtual MeshInfo* BuildMeshInfoFromBuffer(const std::vector<Vertex>& p_buffer, const std::string& p_meshName) = 0;
            /**
                Builds a mesh from the given vector of vertices. Special implementation for skeletalanimation
            */
            virtual MeshInfo* BuildSkeletalMeshInfoFromBuffer(const std::vector<SkeletalVertex>& p_buffer, const std::string& p_meshName) = 0;
            /**
            Returns a pointer to the mesh info for the given mesh name, if no mesh exists a nullptr will be returned
            */
            virtual SpriteInfo* BuildSpriteInfo(SpriteData& p_spriteData) = 0;
            virtual MeshInfo* GetMeshInfo(const std::string& p_meshName) = 0;
            // TODOKO change to acctually load material and not just textures
            virtual MaterialInfo* BuildMaterialInfo(const std::string& p_fileName) = 0;
            virtual MaterialInfo* BuildMaterialInfo(DoremiEditor::Core::MaterialMessage p_materialData) = 0;
            virtual void AddToRenderList(MeshInfo& p_mesh, MaterialInfo& p_material, const DirectX::XMFLOAT4X4& p_orientationMatrix) = 0;
            virtual void AddSpriteToRenderList(SpriteInfo& p_spriteInfo, MaterialInfo& p_material) = 0;
            virtual void Draw() = 0;
        };
    }
}