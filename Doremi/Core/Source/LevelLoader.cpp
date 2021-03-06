/// Game side
#include <LevelLoader.hpp>
#include <EntityComponent/EntityFactory.hpp>
#include <EntityComponent/EntityHandler.hpp>
// Components
#include <EntityComponent/Components/TransformComponent.hpp>
#include <EntityComponent/Components/RenderComponent.hpp>
#include <EntityComponent/Components/TriggerComponent.hpp>
#include <EntityComponent/Components/RigidBodyComponent.hpp>
/// Engine side
#include <DoremiEngine/Core/Include/SharedContext.hpp>
// Graphic
#include <DoremiEngine/Graphic/Include/GraphicModule.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Manager/SubModuleManager.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Manager/MeshManager.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Manager/LightManager.hpp>
// Timing
#include <Timing/TimerManager.hpp>

// Potential Field
#include <Doremi/Core/Include/PotentialFieldGridCreator.hpp>
// Physics

/// DEBUG physics TODOJB remove
#include <DoremiEngine/Physics/Include/PhysicsModule.hpp>
#include <DoremiEngine/Physics/Include/PhysicsMaterialManager.hpp>

#include <DoremiEngine/Physics/Include/RigidBodyManager.hpp>
/// Standard
#include <sstream>
#include <fstream>
#include <iostream>

#include <DoremiEditor/Core/Include/MaterialData.hpp>

namespace Doremi
{
    namespace Core
    {
        LevelLoader::LevelLoader(const DoremiEngine::Core::SharedContext& p_sharedContext) : m_sharedContext(p_sharedContext) {}
        LevelLoader::~LevelLoader() { m_meshes.clear(); }

        void LevelLoader::LoadMaterial(std::ifstream& ifs, int nrMats)
        {
            using namespace DoremiEditor::Core;
            // ladda material

            for(int i = 1; i < nrMats; i++) // defualt material, s� k�r inte hela nrMats TODOXX Why i=1?

            {
                int materialNameSize;
                ifs.read((char*)&materialNameSize, sizeof(int));
                char* materialName = new char[materialNameSize];
                ifs.read((char*)materialName, sizeof(char) * materialNameSize);

                MaterialMessage materialMessage;
                ifs.read((char*)&materialMessage.data.mapMasks, sizeof(int));
                ifs.read((char*)&materialMessage.data.diffuse, sizeof(float));
                ifs.read((char*)&materialMessage.data.color, sizeof(float) * 3);
                ifs.read((char*)&materialMessage.data.ambColor, sizeof(float) * 3);
                ifs.read((char*)&materialMessage.data.specColor, sizeof(float) * 3);
                ifs.read((char*)&materialMessage.data.specCosine, sizeof(float));
                ifs.read((char*)&materialMessage.data.specEccentricity, sizeof(float));
                ifs.read((char*)&materialMessage.data.specRollOff, sizeof(float));

                materialMessage.diffuseTexturePath = "debug.dds";
                materialMessage.glowTexturePath = "glow.dds";

                // load Diffuse texture
                int diffuseTextureNameSize;
                ifs.read((char*)&diffuseTextureNameSize, sizeof(int));
                if(diffuseTextureNameSize != 0)
                {
                    char* diffuseTextureName = new char[diffuseTextureNameSize];
                    ifs.read((char*)diffuseTextureName, sizeof(char) * diffuseTextureNameSize);
                    materialMessage.diffuseTexturePath = diffuseTextureName;
                }

                // Glow texture
                int glowTextureNameSize;
                ifs.read((char*)&glowTextureNameSize, sizeof(int));
                if(glowTextureNameSize != 0)
                {
                    char* glowTextureName = new char[glowTextureNameSize];
                    ifs.read((char*)glowTextureName, sizeof(char) * glowTextureNameSize);
                    materialMessage.glowTexturePath = glowTextureName;
                    // delete glowTextureName;
                }

                materialMessage.nodeName = materialName;
                // TODOKO save the texture name<
                m_materials[materialName] = materialMessage;


                // delete materialName;
            }
        }

        void LevelLoader::LoadMaterialCharacter(std::ifstream& ifs,
                                                int nrMats) // TODOKO SHOULD BE REMOVED LATER ugly hax because character drm dont have glow map yet
        {
            using namespace DoremiEditor::Core;
            // ladda material

            for (int i = 0; i < nrMats; i++) // defualt material, s� k�r inte hela nrMats TODOXX Why i=1?

            {
                int materialNameSize;
                ifs.read((char*)&materialNameSize, sizeof(int));
                char* materialName = new char[materialNameSize];
                ifs.read((char*)materialName, sizeof(char) * materialNameSize);

                MaterialMessage materialMessage;
                ifs.read((char*)&materialMessage.data.mapMasks, sizeof(int));
                ifs.read((char*)&materialMessage.data.diffuse, sizeof(float));
                ifs.read((char*)&materialMessage.data.color, sizeof(float) * 3);
                ifs.read((char*)&materialMessage.data.ambColor, sizeof(float) * 3);
                ifs.read((char*)&materialMessage.data.specColor, sizeof(float) * 3);
                ifs.read((char*)&materialMessage.data.specCosine, sizeof(float));
                ifs.read((char*)&materialMessage.data.specEccentricity, sizeof(float));
                ifs.read((char*)&materialMessage.data.specRollOff, sizeof(float));

                materialMessage.diffuseTexturePath = "debug.dds";
                materialMessage.glowTexturePath = "glow.dds";

                // load Diffuse texture
                int diffuseTextureNameSize;
                ifs.read((char*)&diffuseTextureNameSize, sizeof(int));
                if (diffuseTextureNameSize != 0)
                {
                    char* diffuseTextureName = new char[diffuseTextureNameSize];
                    ifs.read((char*)diffuseTextureName, sizeof(char) * diffuseTextureNameSize);
                    materialMessage.diffuseTexturePath = diffuseTextureName;
                }

                // Glow texture
                int glowTextureNameSize;
                ifs.read((char*)&glowTextureNameSize, sizeof(int));
                if(glowTextureNameSize != 0)
                {
                    char* glowTextureName = new char[glowTextureNameSize];
                    ifs.read((char*)glowTextureName, sizeof(char) * glowTextureNameSize);
                    materialMessage.glowTexturePath = glowTextureName;
                    // delete glowTextureName;
                }

                materialMessage.nodeName = materialName;
                // TODOKO save the texture name<
                m_materials[materialName] = materialMessage;


                // delete materialName;
            }
        }

        void LevelLoader::LoadTransforms(std::ifstream& ifs, int nrTransforms)
        {
            using namespace DoremiEditor::Core;
            for(int i = 0; i < nrTransforms; i++)

            {
                int parentNameSize;
                int transformNameSize;

                ifs.read((char*)&parentNameSize, sizeof(int));
                ifs.read((char*)&transformNameSize, sizeof(int));

                char* parentName = new char[parentNameSize]; // TODOKO Not supported yet
                char* transformName = new char[transformNameSize];

                ifs.read((char*)parentName, sizeof(char) * parentNameSize);
                ifs.read((char*)transformName, sizeof(char) * transformNameSize);

                TransformData transformData;
                ifs.read((char*)&transformData, sizeof(transformData));

                m_transforms[transformName] = transformData;
                delete[] parentName;
                delete[] transformName;
            }
        }

        void LevelLoader::LoadTransformsCharacter(std::ifstream& ifs, int nrTransforms) // TODOKO Remove and replase when cahracte have own format
        {
            using namespace DoremiEditor::Core;
            for(int i = 0; i < nrTransforms; i++)
            {
                int parentNameSize;
                int transformNameSize;

                ifs.read((char*)&parentNameSize, sizeof(int));
                ifs.read((char*)&transformNameSize, sizeof(int));

                char* parentName = new char[parentNameSize]; // TODOKO Not supported yet
                char* transformName = new char[transformNameSize];

                ifs.read((char*)parentName, sizeof(char) * parentNameSize);
                ifs.read((char*)transformName, sizeof(char) * transformNameSize);

                CharacterTransformData transformData;
                ifs.read((char*)&transformData, sizeof(transformData));

                m_transformsCharacter[transformName] = transformData;
                delete[] parentName;
                delete[] transformName;
            }
        }

        void LevelLoader::LoadMeshes(std::ifstream& ifs, int nrMeshes)
        {
            using namespace DoremiEditor::Core;
            // ladda meshes. TODOSH Fixa s� samma mesh itne l�ses in flera g�nger, allts� s� att samma mesh data inte finns p� 2 st�llen p� GPU
            for(int i = 0; i < nrMeshes; i++)
            {
                int transformNameSize;
                int meshNameSize;
                int nrOfTransforms;

                ifs.read((char*)&nrOfTransforms, sizeof(int));
                std::vector<char*> transformNames;
                char* transformName = nullptr;

                for(int t = 0; t < nrOfTransforms; t++) // l�ser in alla transforms f�r meshen, blir flera om instanciering skall anv�ndas
                {
                    ifs.read((char*)&transformNameSize, sizeof(int));
                    transformName = new char[transformNameSize];

                    ifs.read((char*)transformName, sizeof(char) * transformNameSize);
                    transformNames.push_back(transformName);
                }


                ifs.read((char*)&meshNameSize, sizeof(int));
                char* meshName = new char[meshNameSize];

                ifs.read((char*)meshName, sizeof(char) * meshNameSize);


                int materialNameSize;
                ifs.read((char*)&materialNameSize, sizeof(int));

                char* materialName = new char[materialNameSize];
                ifs.read((char*)materialName, sizeof(char) * materialNameSize);


                // messageSTART****
                int meshID;
                ifs.read((char*)&meshID, sizeof(int));

                m_meshes.emplace(meshName, MeshData());
                MeshData& meshData = m_meshes[meshName];

                ifs.read((char*)&meshData.vertCount, sizeof(int));
                ifs.read((char*)&meshData.normalCount, sizeof(int));
                ifs.read((char*)&meshData.UVCount, sizeof(int));
                ifs.read((char*)&meshData.indCount, sizeof(int));
                ifs.read((char*)&meshData.triCount, sizeof(int));

                meshData.positions = new XMFLOAT3[meshData.vertCount];
                meshData.normals = new XMFLOAT3[meshData.normalCount];
                meshData.uvs = new XMFLOAT2[meshData.UVCount];

                meshData.indexPositions = new int[meshData.indCount];
                meshData.indexNormals = new int[meshData.indCount];
                meshData.indexUVs = new int[meshData.indCount];
                meshData.trianglesPerFace = new int[meshData.triCount];
                ifs.read((char*)meshData.positions, sizeof(XMFLOAT3) * meshData.vertCount);
                ifs.read((char*)meshData.normals, sizeof(XMFLOAT3) * meshData.normalCount);
                ifs.read((char*)meshData.uvs, sizeof(XMFLOAT2) * meshData.UVCount);

                ifs.read((char*)meshData.indexPositions, sizeof(int) * meshData.indCount);
                ifs.read((char*)meshData.indexNormals, sizeof(int) * meshData.indCount);
                ifs.read((char*)meshData.indexUVs, sizeof(int) * meshData.indCount);
                ifs.read((char*)meshData.trianglesPerFace, sizeof(int) * meshData.triCount);

                // All the transform that this mesh should be placed at and puts it in the coupling vector
                for(size_t i = 0; i < nrOfTransforms; i++)
                {
                    m_meshCoupling.push_back(ObjectCouplingInfo(transformNames[i], std::string(meshName), std::string(materialName)));
                }

                // Cleanup
                delete[] meshName;
                delete[] materialName;
                for(auto& i : transformNames)
                {
                    delete i;
                }
                transformNames.clear();
            }
        }

        void LevelLoader::LoadLights(std::ifstream& ifs, int nrLights)
        {
            using namespace std;
            using namespace DoremiEditor::Core;
            m_lights.reserve(nrLights);

            for(int i = 0; i < nrLights; i++)

            {
                int transformNameSize;
                int lightNameSize;

                ifs.read((char*)&transformNameSize, sizeof(int));
                ifs.read((char*)&lightNameSize, sizeof(int));
                LightData lightData;

                char* transformName = new char[transformNameSize];
                char* lightName = new char[lightNameSize];

                ifs.read((char*)transformName, sizeof(char) * transformNameSize);
                ifs.read((char*)lightName, sizeof(char) * lightNameSize);
                ifs.read((char*)&lightData.type, sizeof(int));
                ifs.read((char*)&lightData.decayType, sizeof(int));
                ifs.read((char*)&lightData.intensity, sizeof(float));
                ifs.read((char*)&lightData.colorDiffuse, sizeof(XMFLOAT3));
                ifs.read((char*)&lightData.direction, sizeof(XMFLOAT3));
                ifs.read((char*)&lightData.dropOff, sizeof(float));
                ifs.read((char*)&lightData.coneAngle, sizeof(float));
                ifs.read((char*)&lightData.penumAgle, sizeof(float));
                m_lights.push_back(move(lightData));

                m_lightNames[i] = pair<string, string>(string(transformName), string(lightName));

                delete[] transformName;
                delete[] lightName;
            }
        }

        void LevelLoader::BuildEntities()
        {
            const size_t length = m_meshCoupling.size();
            for(size_t i = 0; i < length; i++)
            {
                // Get scale, pos and transform
                const DirectX::XMFLOAT3 scale = m_transforms[m_meshCoupling[i].transformName].scale;
                m_currentOrientation = m_transforms[m_meshCoupling[i].transformName].rotation;
                m_currentPos = m_transforms[m_meshCoupling[i].transformName].translation;

                // Build vertex list for graphics, and position and index list for physics
                std::vector<XMFLOAT3> positionPX;
                std::vector<int> indexPX;
                std::vector<DoremiEngine::Graphic::Vertex>& vertexBuffer =
                    ComputeVertexAndPositionAndIndex(m_meshes[m_meshCoupling[i].meshName], scale, positionPX, indexPX);

                // Create Entity
                const int entityID = EntityFactory::GetInstance()->CreateEntity(Blueprints::EmptyEntity);

                // Create components
                bool shouldBuildPhysics = BuildComponents(entityID, i, vertexBuffer);
                if(shouldBuildPhysics)
                {
                    SetPhysicalAttributesOnMesh(entityID, positionPX, indexPX);
                }
            }
        }

        void LevelLoader::LoadTriggers()
        {
            //// Triggering ;) TODOKO should only be done on server and the server should send the events to the client. Event sending is not
            /// supported
            //// yet though
            // int entityIDTrigger = EntityFactory::GetInstance()->CreateEntity(Blueprints::TriggerEntity);
            // EntityHandler::GetInstance().AddComponent(entityIDTrigger, (int)ComponentType::Trigger | (int)ComponentType::Transform |
            // (int)ComponentType::RigidBody);

            //// Transform for trigger
            // TransformComponent* transComp = EntityHandler::GetInstance().GetComponentFromStorage<TransformComponent>(entityIDTrigger);
            // transComp->position = XMFLOAT3(-420.4f, 151.5f, -110.3f);
            // transComp->rotation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

            //// The actual trigger component
            // TriggerComponent* triggComp = EntityHandler::GetInstance().GetComponentFromStorage<TriggerComponent>(entityIDTrigger);
            // triggComp->dimensions = XMFLOAT3(7.0f, 10.0f, 34.0f);
            // triggComp->triggerType = TriggerType::GoalTrigger;

            //// Create material
            // int materialTriggID = m_sharedContext.GetPhysicsModule().GetPhysicsMaterialManager().CreateMaterial(0.0f, 0.0f, 0.0f);
        }


        std::vector<DoremiEngine::Graphic::Vertex>
        LevelLoader::ComputeVertexAndPositionAndIndex(const DoremiEditor::Core::MeshData& p_data, const DirectX::XMFLOAT3& p_scale,
                                                      std::vector<DirectX::XMFLOAT3>& o_positionPX, std::vector<int>& o_indexPX)

        {
            vector<DoremiEngine::Graphic::Vertex> vertexBuffer;

            const XMMATRIX scaleMat = XMMatrixScalingFromVector(XMLoadFloat3(&p_scale));
            DoremiEngine::Graphic::Vertex tempV;
            XMVECTOR posVec;
            XMFLOAT3 scaledPos;

            for(int i = 0; i < p_data.indCount; i += 3)

            {
                //// First
                // First vertex
                tempV.position = p_data.positions[p_data.indexPositions[i + 2]];
                tempV.position.z = tempV.position.z * -1.0f;
                tempV.normal = p_data.normals[p_data.indexNormals[i + 2]];
                tempV.normal.z = tempV.normal.z * -1.0f;
                tempV.textureCoordinate = p_data.uvs[p_data.indexUVs[i + 2]];
                tempV.textureCoordinate.y = tempV.textureCoordinate.y * -1.0f;
                vertexBuffer.push_back(tempV);

                // First index
                posVec = XMLoadFloat3(&tempV.position);
                posVec = XMVector3Transform(posVec, scaleMat);
                XMStoreFloat3(&scaledPos, posVec);
                o_positionPX.push_back(scaledPos);
                o_indexPX.push_back(i);

                //// Second
                // Second vertex
                tempV.position = p_data.positions[p_data.indexPositions[i + 1]];
                tempV.position.z = tempV.position.z * -1.0f;
                tempV.normal = p_data.normals[p_data.indexNormals[i + 1]];
                tempV.normal.z = tempV.normal.z * -1.0f;
                tempV.textureCoordinate = p_data.uvs[p_data.indexUVs[i + 1]];
                tempV.textureCoordinate.y = tempV.textureCoordinate.y * -1.0f;
                vertexBuffer.push_back(tempV);

                // Second index
                posVec = XMLoadFloat3(&tempV.position);
                posVec = XMVector3Transform(posVec, scaleMat);
                XMStoreFloat3(&scaledPos, posVec);
                o_positionPX.push_back(scaledPos);
                o_indexPX.push_back(i + 1);

                //// Third
                // Third vertex
                tempV.position = p_data.positions[p_data.indexPositions[i]];
                tempV.position.z = tempV.position.z * -1.0f;
                tempV.normal = p_data.normals[p_data.indexNormals[i]];
                tempV.normal.z = tempV.normal.z * -1.0f;
                tempV.textureCoordinate = p_data.uvs[p_data.indexUVs[i]];
                tempV.textureCoordinate.y = tempV.textureCoordinate.y * -1.0f;
                vertexBuffer.push_back(tempV);

                // Third index
                posVec = XMLoadFloat3(&tempV.position);
                posVec = XMVector3Transform(posVec, scaleMat);
                XMStoreFloat3(&scaledPos, posVec);
                o_positionPX.push_back(scaledPos);
                o_indexPX.push_back(i + 2);
            }
            return vertexBuffer;
        }

        // TODORT Boths vectors should be const
        void LevelLoader::SetPhysicalAttributesOnMesh(int p_entityID, std::vector<DirectX::XMFLOAT3>& p_positionPX, std::vector<int>& p_indexPX)
        {
            using namespace DoremiEngine::Physics;
            PhysicsModule& physicsModule = m_sharedContext.GetPhysicsModule();
            RigidBodyManager& rigidBodyManager = physicsModule.GetRigidBodyManager();
            PhysicsMaterialManager& physicsMaterialManager = physicsModule.GetPhysicsMaterialManager();

            const int material = physicsMaterialManager.CreateMaterial(0.5, 0.5, 0.5);
            rigidBodyManager.AddMeshBodyStatic(p_entityID, m_currentPos, m_currentOrientation, p_positionPX, p_indexPX, material);
            rigidBodyManager.SetDrain(p_entityID, true);

            // Add component
            EntityHandler::GetInstance().AddComponent(p_entityID, static_cast<uint32_t>(ComponentType::RigidBody));
            GetComponent<RigidBodyComponent>(p_entityID)->geometry = RigidBodyGeometry::mesh;

            /*
            TODOJB TODOXX haxy callback filtering. Basically we want the kinematic objects (elevators)
            to ignore the world (every static mesh) and each other. Specifically:
            All objects have the bitmask 0001 and ignore nothing
            Kinematic objects have the bitmask 0011 and ignore the bit 0010
            Static objects have the bitmask 0011 and ignore nothing.
            Thus, kinematic object ignores each other, and ignore the world,
            but collides with everything else (enemies, projectiles etc.)
            EDIT!! NO LONGER VALID. The method still works tho*/
            rigidBodyManager.SetCallbackFiltering(p_entityID, 1, 0, 0, 0);
        }


        void LevelLoader::CalculateAABBBoundingBox(const std::vector<DoremiEngine::Graphic::Vertex>& p_vertexBuffer,
                                                   const DoremiEditor::Core::TransformData& p_transformationData, DirectX::XMFLOAT3& o_max,
                                                   DirectX::XMFLOAT3& o_min, DirectX::XMFLOAT3& o_center)
        {
            // Max and min that a int can have
            int maxInt = std::numeric_limits<int>::max();
            int minInt = std::numeric_limits<int>::min();
            DirectX::XMFLOAT3 maxPosition = DirectX::XMFLOAT3(minInt, minInt, minInt); // THe lowest possible position a int can have
            DirectX::XMFLOAT3 minPosition = DirectX::XMFLOAT3(maxInt, maxInt, maxInt); // The highest possible position a int can have
            DirectX::XMVECTOR rotation = DirectX::XMLoadFloat4(&p_transformationData.rotation);
            DirectX::XMMATRIX translation = XMMatrixTranslationFromVector(XMLoadFloat3(&p_transformationData.translation));
            DirectX::XMMATRIX scale = XMMatrixScalingFromVector(XMLoadFloat3(&p_transformationData.scale));
            size_t length = p_vertexBuffer.size();
            for(size_t i = 0; i < length; i++)
            {
                // Rotate positions
                DirectX::XMVECTOR positionVec = XMLoadFloat3(&p_vertexBuffer[i].position);

                positionVec = XMVector3Transform(positionVec, scale);
                positionVec = XMVector3Rotate(positionVec, rotation);
                positionVec = XMVector3Transform(positionVec, translation);


                XMFLOAT3 rotatedPos;
                XMStoreFloat3(&rotatedPos, positionVec);
                // Finding max value

                if(rotatedPos.x > maxPosition.x)
                {
                    maxPosition.x = rotatedPos.x;
                }
                if(rotatedPos.y > maxPosition.y)
                {
                    maxPosition.y = rotatedPos.y;
                }
                if(rotatedPos.z > maxPosition.z)
                {
                    maxPosition.z = rotatedPos.z;
                }

                // FInding min value
                if(rotatedPos.x < minPosition.x)
                {
                    minPosition.x = rotatedPos.x;
                }
                if(rotatedPos.y < minPosition.y)
                {
                    minPosition.y = rotatedPos.y;
                }
                if(rotatedPos.z < minPosition.z)
                {
                    minPosition.z = rotatedPos.z;
                }
            }
            // Max and min are now centered around origo with no scale and no rotation...
            DirectX::XMVECTOR maxVector = XMLoadFloat3(&maxPosition);
            DirectX::XMVECTOR minVector = XMLoadFloat3(&minPosition);
            // Create scale and translation matrices


            // scale first then translate the points
            // maxVector = XMVector3Transform(maxVector, scale);
            // maxVector = XMVector3Transform(maxVector, translation);
            // minVector = XMVector3Transform(minVector, scale);
            // minVector = XMVector3Transform(minVector, translation);

            // The center is taken from the heighest point and the lowest point
            DirectX::XMFLOAT3 centerPoint;
            XMStoreFloat3(&o_center, (maxVector + minVector) / 2);
            XMStoreFloat3(&o_max, maxVector);
            XMStoreFloat3(&o_min, minVector);
        }
        void LevelLoader::CalculateOBBoundingBox(std::vector<DoremiEngine::Graphic::Vertex>& p_vertexBuffer, const DoremiEditor::Core::TransformData & p_transformationData, DirectX::XMFLOAT3 & o_max, DirectX::XMFLOAT3 & o_min, DirectX::XMFLOAT3 & o_center)
        {
            // Max and min that a int can have
            int maxInt = std::numeric_limits<int>::max();
            int minInt = std::numeric_limits<int>::min();
            DirectX::XMFLOAT3 maxPosition = DirectX::XMFLOAT3(minInt, minInt, minInt); // THe lowest possible position a int can have
            DirectX::XMFLOAT3 minPosition = DirectX::XMFLOAT3(maxInt, maxInt, maxInt); // The highest possible position a int can have
            //DirectX::XMVECTOR rotation = DirectX::XMLoadFloat4(&p_transformationData.rotation);
            DirectX::XMMATRIX translation = XMMatrixTranslationFromVector(XMLoadFloat3(&p_transformationData.translation));
            DirectX::XMMATRIX scale = XMMatrixScalingFromVector(XMLoadFloat3(&p_transformationData.scale));
            size_t length = p_vertexBuffer.size();
            for (size_t i = 0; i < length; i++)
            {
                // Rotate positions
                DirectX::XMVECTOR positionVec = XMLoadFloat3(&p_vertexBuffer[i].position);

                //positionVec = XMVector3Transform(positionVec, scale);
                //positionVec = XMVector3Rotate(positionVec, rotation);
                //positionVec = XMVector3Transform(positionVec, translation);


                XMFLOAT3 rotatedPos;
                XMStoreFloat3(&rotatedPos, positionVec);
                // Finding max value

                if (rotatedPos.x > maxPosition.x)
                {
                    maxPosition.x = rotatedPos.x;
                }
                if (rotatedPos.y > maxPosition.y)
                {
                    maxPosition.y = rotatedPos.y;
                }
                if (rotatedPos.z > maxPosition.z)
                {
                    maxPosition.z = rotatedPos.z;
                }

                // FInding min value
                if (rotatedPos.x < minPosition.x)
                {
                    minPosition.x = rotatedPos.x;
                }
                if (rotatedPos.y < minPosition.y)
                {
                    minPosition.y = rotatedPos.y;
                }
                if (rotatedPos.z < minPosition.z)
                {
                    minPosition.z = rotatedPos.z;
                }
            }
            // Max and min are now centered around origo with no scale and no rotation...
            DirectX::XMVECTOR maxVector = XMLoadFloat3(&maxPosition);
            DirectX::XMVECTOR minVector = XMLoadFloat3(&minPosition);
            // Create scale and translation matrices


            // scale first then translate the points
            // maxVector = XMVector3Transform(maxVector, scale);
            // maxVector = XMVector3Transform(maxVector, translation);
            // minVector = XMVector3Transform(minVector, scale);
            // minVector = XMVector3Transform(minVector, translation);

            // The center is taken from the heighest point and the lowest point
            DirectX::XMFLOAT3 centerPoint;
            XMStoreFloat3(&o_center, (maxVector + minVector) / 2);

            minVector = XMVector3Transform(minVector, scale);            
            maxVector = XMVector3Transform(maxVector, scale);
            
            XMStoreFloat3(&o_max, maxVector);
            XMStoreFloat3(&o_min, minVector);

            for (size_t i = 0; i < length; i++)
            {                
                p_vertexBuffer[i].position.x -= o_center.x;
                p_vertexBuffer[i].position.y -= o_center.y;
                p_vertexBuffer[i].position.z -= o_center.z;
            }
        }
    }
}
