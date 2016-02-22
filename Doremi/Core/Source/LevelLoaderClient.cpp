/// Game side
#include <LevelLoaderClient.hpp>
#include <EntityComponent/EntityHandler.hpp>
// Components
#include <EntityComponent/Components/TransformComponent.hpp>
#include <EntityComponent/Components/RenderComponent.hpp>
#include <EntityComponent/Components/TriggerComponent.hpp>
#include <EntityComponent/Components/RigidBodyComponent.hpp>
#include <EntityComponent/Components/PhysicsMaterialComponent.hpp>
/// Engine side
#include <DoremiEngine/Core/Include/SharedContext.hpp>
// Graphic
#include <DoremiEngine/Graphic/Include/GraphicModule.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Manager/SubModuleManager.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Manager/MeshManager.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Manager/LightManager.hpp>
#include <DoremiEngine/Graphic/Include/Interface/Animation/SkeletalInformation.hpp>
// Timing
#include <DoremiEngine/Timing/Include/Measurement/TimeMeasurementManager.hpp>
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
using namespace std;
namespace Doremi
{
    namespace Core
    {
        LevelLoaderClient::LevelLoaderClient(const DoremiEngine::Core::SharedContext& p_sharedContext) : LevelLoader(p_sharedContext) {}

        LevelLoaderClient::~LevelLoaderClient() {}

        void LevelLoaderClient::LoadLevel(const std::string& p_fileName)
        {
            // TODOXX WTF, this function must be restructured.
            using namespace std;
            const string fileName = m_sharedContext.GetWorkingDirectory() + p_fileName;
            ifstream ifs;
            ifs.open(fileName, ifstream::in | ofstream::binary);
            if(ifs.is_open() == true)
            {
                // testa o l�sa lite

                // scene name
                int sceneNameSize;
                ifs.read((char*)&sceneNameSize, sizeof(int));
                char* sceneName = new char[sceneNameSize];
                ifs.read((char*)sceneName, sizeof(char) * sceneNameSize);

                // how much different stuff there is
                int nrMats, nrTransforms, nrMeshes, nrLights;
                ifs.read((char*)&nrMats, sizeof(int));
                ifs.read((char*)&nrTransforms, sizeof(int));
                ifs.read((char*)&nrMeshes, sizeof(int));
                ifs.read((char*)&nrLights, sizeof(int));

                LoadMaterial(ifs, nrMats);
                LoadTransforms(ifs, nrTransforms);
                LoadMeshes(ifs, nrMeshes);
                LoadLights(ifs, nrLights);
                BuildEntities();
                LoadTriggers();

                BuildLights();
            }
            else
            {
                // TODOXX somethings wrong
            }
        }

        CharacterDataNames LevelLoaderClient::LoadSkeletalCharacter(const std::string& p_fileName,
                                                                    DoremiEngine::Graphic::SkeletalInformation& p_upperBodySkeletalInformation,
                                                                    DoremiEngine::Graphic::SkeletalInformation& p_lowerBodySkeletalInformation)
        {
            // Get the full path
            string fileName = m_sharedContext.GetWorkingDirectory() + p_fileName;

            ifstream ifs;
            ifs.open(fileName, ifstream::in | ifstream::binary);
            if(ifs.is_open() == true)
            {
                // Read the Name of the skeletalanimationmodel (This is set in maya, I use the name to have an identifier for meshname)
                int sceneNameSize;
                ifs.read((char*)&sceneNameSize, sizeof(int));
                char* sceneName = new char[sceneNameSize];
                ifs.read((char*)sceneName, sizeof(char) * sceneNameSize);
                std::string t_sceneName = sceneName;

                // Read how many joints, materials, transforms and meshes we will read. Used for forloops
                int nrJoints, nrMats, nrTransforms, nrMeshes;
                ifs.read((char*)&nrJoints, sizeof(int));
                ifs.read((char*)&nrMats, sizeof(int));
                ifs.read((char*)&nrTransforms, sizeof(int));
                ifs.read((char*)&nrMeshes, sizeof(int));

                // Skapa listorna f�r jointheirarki: En f�r �verkroppen och en f�r underkroppen. Sam skapa 2 maps f�r animationer
                // The heirachy of the skeleton. Index = jointID. Value = ParentID
                std::vector<int> t_upperBodyJointHeirarchy;
                std::vector<int> t_lowerBodyJointHeirarchy;
                // All the animations this is used as final map and used in skeletalanimations
                std::map<std::string, DoremiEngine::Graphic::AnimationClip> t_upperBodyAnimations;
                std::map<std::string, DoremiEngine::Graphic::AnimationClip> t_lowerBodyAnimations;

                // L�s animationsinformation. Inneh�ller information som beh�vs f�r jointinformationsladdningen
                // Read how many animation this file contains
                int t_nrAnimations;
                ifs.read((char*)&t_nrAnimations, sizeof(int));
                std::vector<AnimationInformation> t_animationInformations(t_nrAnimations);
                // Load AnimationInformation
                t_animationInformations = LoadAnimationInformation(ifs, t_nrAnimations);
                std::vector<std::string> t_animationNames(t_nrAnimations);

                /// Ladda jointanimationerna
                // Temporary savings of the animations.
                std::vector<DoremiEngine::Graphic::AnimationClip> t_upperBodyAnimationVector(t_nrAnimations);
                std::vector<DoremiEngine::Graphic::AnimationClip> t_lowerBodyAnimationVector(t_nrAnimations);
                // Load the jointAnimations. We save the jointHeirarchy as outparameters in t_lowerBodyJointHeirachy and t_upperBodyJointHeirachy
                // We also save the animationVectors for both upper and lower body in the outparameter t_upperBodyAnimationVector and
                // t_lowerBodyAnimationVector.
                LoadJointAnimations(ifs, nrJoints, t_nrAnimations, t_upperBodyAnimationVector, t_lowerBodyAnimationVector, t_animationInformations,
                                    t_lowerBodyJointHeirarchy, t_upperBodyJointHeirarchy, t_animationNames);
                // S�tt �ver animationsvectorerna till mapsen som anv�nds i skelettanimationerna. M�ste anv�nda vectorer inna f�r att l�sa i r�tt
                // ordning.
                for(size_t i = 0; i < t_nrAnimations; i++)
                {
                    t_upperBodyAnimations.emplace(t_animationNames[i], t_upperBodyAnimationVector[i]);
                    t_lowerBodyAnimations.emplace(t_animationNames[i], t_lowerBodyAnimationVector[i]);
                }
                // materialen kommer senare att l�sas in h�r imellan

                /// L�sa transformsen
                // En map f�r att kunna anv�nda r�tt transformationsmatris till r�tt mesh.
                std::map<std::string, XMFLOAT4X4> t_transformMap;
                // Load the transforms and place them in this map
                t_transformMap = LoadSkeletalTransforms(ifs, nrTransforms);

                /// L�s Skelettvertexbuffern
                // This buffer is used to combine all the meshes into one. And convert the vertexstruct to skeletalvertex from the computevertex thing
                vector<DoremiEngine::Graphic::SkeletalVertex> t_skeletalVertexBuffer;
                // Load the vertices and place them in this buffer
                t_skeletalVertexBuffer = LoadSkeletalMesh(ifs, nrMeshes, t_transformMap);

                // Vanliga buildcalls f�r att f� en mesh � material
                DoremiEngine::Graphic::MeshManager& meshManager = m_sharedContext.GetGraphicModule().GetSubModuleManager().GetMeshManager();
                meshManager.BuildSkeletalMeshInfoFromBuffer(t_skeletalVertexBuffer, t_sceneName);
                meshManager.BuildMaterialInfo(m_materials[m_meshCoupling[0].materialName]);
                // Detta �r typen vi anv�nder f�r att sedan loada mesh/material i templatecreator
                CharacterDataNames o_charData;
                o_charData.meshName = t_sceneName;
                o_charData.materialName = m_materials[m_meshCoupling[0].materialName].diffuseTextureName;
                ifs.close();

                // S�tt v�rdena som beh�vs f�r skelettanimationens keyframeuppdatering. (Basicly datan som anv�nds vid animationen)
                p_upperBodySkeletalInformation.Set(t_upperBodyJointHeirarchy, t_upperBodyAnimations);
                p_lowerBodySkeletalInformation.Set(t_lowerBodyJointHeirarchy, t_lowerBodyAnimations);
                return o_charData;
            }
        }

        std::vector<LevelLoaderClient::AnimationInformation> LevelLoaderClient::LoadAnimationInformation(std::ifstream& ifs, const int& p_nrOfAnimationInformations)
        {
            std::vector<AnimationInformation> t_animationInformations(p_nrOfAnimationInformations);
            for(size_t i = 0; i < p_nrOfAnimationInformations; i++)
            {
                int t_nameSize;
                AnimationInformation t_animationInfo;
                ifs.read((char*)&t_nameSize, sizeof(int));
                char* t_animationName = new char[t_nameSize];
                ifs.read((char*)t_animationName, sizeof(char) * t_nameSize);
                std::string t_animationString(t_animationName);
                t_animationInfo.name = t_animationString;
                delete t_animationName;
                ifs.read((char*)&t_animationInfo.startFrame, sizeof(int));
                ifs.read((char*)&t_animationInfo.endFrame, sizeof(int));
                ifs.read((char*)&t_animationInfo.prioPart, sizeof(int));
                float maxTime;
                ifs.read((char*)&maxTime, sizeof(float));
                t_animationInformations[i] = t_animationInfo;
            }
            return t_animationInformations;
        }

        void LevelLoaderClient::LoadJointAnimations(std::ifstream& ifs, const int& p_nrOfJoints, const int& p_nrOfAnimations,
                                                    std::vector<DoremiEngine::Graphic::AnimationClip>& o_upperBodyAnimationVector,
                                                    std::vector<DoremiEngine::Graphic::AnimationClip>& o_lowerBodyAnimationVector,
                                                    std::vector<AnimationInformation> p_animationInformations, std::vector<int>& o_lowerBodyJointHeirarchy,
                                                    std::vector<int>& o_upperBodyJointHeirarchy, std::vector<std::string>& o_animationNames)
        {
            // Per joint
            int nrKeyFrames;
            for(int i = 0; i < p_nrOfJoints; i++)
            {
                int ID, parentID; // joint IDs
                int nrChildrenTransforms;
                // int nrKeyFrames;
                int nameSize;
                int bodyPartID;
                // Read ID parentID nrChildrenTransforms, nrKeyframes and How long the name is
                ifs.read((char*)&ID, sizeof(int));
                ifs.read((char*)&parentID, sizeof(int));
                ifs.read((char*)&nrChildrenTransforms, sizeof(int));
                ifs.read((char*)&nrKeyFrames, sizeof(int));
                ifs.read((char*)&bodyPartID, sizeof(int));
                ifs.read((char*)&nameSize, sizeof(int));


                char* name = new char[nameSize]; // namnet p� jointen
                ifs.read((char*)name, sizeof(char) * nameSize);
                if(bodyPartID == 1)
                {
                    // Om det �r upperbody s� �re bara att pusha in eftersom dessa kommer f�rst
                    o_upperBodyJointHeirarchy.push_back(parentID);
                }
                else if(bodyPartID == 2)
                {
                    if(parentID == 0)
                    {
                        // M�ste fortfarande spara ner rotnoden f�r att kunna r�kna ut r�tt i finaltransforms. Denna bortser vi dock n�r vi l�gger
                        // ihop matrisvektorerna
                        o_lowerBodyJointHeirarchy.push_back(parentID);
                    }
                    else
                    {
                        // M�ste byta ut parentIDn som kommer fr�n maya eftersom den f�rv�ntar sig et thelt skelett. D�rf�r tar vi parentnumret och
                        // subtraherar med antalet joints i �vre kropppen och en extra pga
                        // att vi anv�nder rootnoden i b�de lower och upper bodyn.
                        o_lowerBodyJointHeirarchy.push_back(parentID - (o_upperBodyJointHeirarchy.size() - 1));
                    }
                }
                else if(bodyPartID == 0)
                {
                    // L�gger till rotnoden i b�da listorna
                    o_upperBodyJointHeirarchy.push_back(parentID);
                    o_lowerBodyJointHeirarchy.push_back(parentID);
                }

                for(int y = 0; y < nrChildrenTransforms;
                    y++) // skriv alla transforms (f�rutom joints) som �r children till denna joint, dvs transforms till meshes
                {
                    // Detta har ingen funktion atm men kan inte ta bort reads d� fakkarprogrammet TODOLH ta bort.
                    int childNameSize;
                    ifs.read((char*)&childNameSize, sizeof(int));
                    char* childTransformName = new char[childNameSize]; // namn p� transform som skall vara till mesh, dvs INGEN joint
                    ifs.read((char*)childTransformName, sizeof(char) * childNameSize);
                    delete childTransformName;
                }

                // SPara ner keyframesen i boneanimaiton som sedan sparas ner i animationclip � mappas mot animationclipnamnet
                DoremiEngine::Graphic::BoneAnimation t_boneAnimation;
                // TODOLH TODOSH ta bort skcikning och l�sning av jointquaternion
                XMFLOAT4 t_jointQuaternion;
                ifs.read((char*)&t_jointQuaternion, sizeof(float) * 4); // jointorientationen m�ste anv�ndas f�r att f� det r�tt...
                // Loopa �ver keyframes � spar ner i t_boneAnimations. Detta �r en tempor�r variabel eftersom den bara sparas undan i
                // animationsclippet och tas fram efter timeposen den har
                int t_animationInformationIndex = 0;
                for(int y = 0; y < nrKeyFrames; y++)
                {
                    for(size_t p = 0; p < p_nrOfAnimations; p++)
                    {
                        if(y >= p_animationInformations[p].startFrame && y < p_animationInformations[p].endFrame) //>= ?
                        {
                            if(t_animationInformationIndex != p)
                            {
                                // Nu �r alla keyframes f�r denna benets fulla animation sparat. D� ska benanimationsdatan sparas undan i
                                // animationsclippet. D�r sparas alla dessa benanimationer
                                if(bodyPartID == 1)
                                {
                                    o_upperBodyAnimationVector[p - 1].BoneAnimations.push_back(t_boneAnimation);
                                }
                                else if(bodyPartID == 2)
                                {
                                    o_lowerBodyAnimationVector[p - 1].BoneAnimations.push_back(t_boneAnimation);
                                }
                                else if(bodyPartID == 0)
                                {
                                    o_upperBodyAnimationVector[p - 1].BoneAnimations.push_back(t_boneAnimation);
                                    o_lowerBodyAnimationVector[p - 1].BoneAnimations.push_back(t_boneAnimation);
                                }
                                // t_animationVector[p-1].BoneAnimations.push_back(t_boneAnimation);
                                // S�tts m�nga g�nger i on�dan...
                                o_animationNames[p - 1] = p_animationInformations[p - 1].name;
                                t_boneAnimation = DoremiEngine::Graphic::BoneAnimation();
                                t_animationInformationIndex = p;
                                break;
                            }
                            else
                            {
                                break;
                            }
                        }
                        else
                        {
                            // Do nothing
                        }
                    }
                    DoremiEngine::Graphic::KeyFrame t_keyFrameTemp;
                    // Tempor�r t_frame f�r att anv�ndas till utr�kning av hastighet av animation.
                    int t_frame;
                    // ska tas bort under
                    XMFLOAT3 t_eulerAngles;
                    ifs.read((char*)&t_keyFrameTemp.position, sizeof(float) * 3);
                    ifs.read((char*)&t_keyFrameTemp.quaternion, sizeof(float) * 4);
                    ifs.read((char*)&t_eulerAngles, sizeof(float) * 3);
                    ifs.read((char*)&t_keyFrameTemp.scale, sizeof(float) * 3); // TODOLH TODOSH b�r ta bort antingen euler eller quaternion. De
                    // ska va likadana � fylla samma funktion
                    ifs.read((char*)&t_frame, sizeof(int));
                    t_frame -= p_animationInformations[t_animationInformationIndex].startFrame;
                    // H�rdkodat v�rde atm. B�r komma fr�n maya (?) TODOLH. B�r iaf inte vara en variabel h�r
                    float t_timeMax = 0.5f;
                    // R�kna ut vilken timestamp som ska s�ttas p� denna frame.
                    float t_currentTime = (t_timeMax / float(nrKeyFrames)) * t_frame;
                    t_keyFrameTemp.time = t_currentTime;
                    // Spara ner keyframedatan i benanimationsscructen som sedan pushbackas in i animationclipets benanimationsvector utanf�r
                    // forloopen
                    t_boneAnimation.Keyframes.push_back(t_keyFrameTemp);
                    if((nrKeyFrames - 1) == y)
                    {
                        // Nu �r alla keyframes f�r denna benets fulla animation sparat. D� ska benanimationsdatan sparas undan i
                        // animationsclippet. D�r sparas alla dessa benanimationer
                        if(bodyPartID == 1)
                        {
                            o_upperBodyAnimationVector[t_animationInformationIndex].BoneAnimations.push_back(t_boneAnimation);
                        }
                        else if(bodyPartID == 2)
                        {
                            o_lowerBodyAnimationVector[t_animationInformationIndex].BoneAnimations.push_back(t_boneAnimation);
                        }
                        else if(bodyPartID == 0)
                        {
                            o_upperBodyAnimationVector[t_animationInformationIndex].BoneAnimations.push_back(t_boneAnimation);
                            o_lowerBodyAnimationVector[t_animationInformationIndex].BoneAnimations.push_back(t_boneAnimation);
                        }
                        // t_animationVector[t_animationInformationIndex].BoneAnimations.push_back(t_boneAnimation);
                        // S�tts m�nga g�nger i on�dan...
                        o_animationNames[t_animationInformationIndex] = p_animationInformations[t_animationInformationIndex].name;
                        t_boneAnimation = DoremiEngine::Graphic::BoneAnimation();
                    }
                }

                delete name; // name isnt used atm
            }
        }

        std::map<std::string, DirectX::XMFLOAT4X4> LevelLoaderClient::LoadSkeletalTransforms(std::ifstream& ifs, const int& p_nrOfTransforms)
        {
            std::map<std::string, DirectX::XMFLOAT4X4> t_transformMap;
            for(int i = 0; i < p_nrOfTransforms; i++)
            {

                int parentNameSize;
                int transformNameSize;

                // Read namelengths of parent(not used) and transform
                ifs.read((char*)&parentNameSize, sizeof(int));
                ifs.read((char*)&transformNameSize, sizeof(int));

                // Read parent name(not used) and transformname
                char* parentName = new char[parentNameSize]; // denna anv�nds inte alls f�r tillf�llet men kan komma att implementeras i
                // framtiden! transformsen (f�r meshar) ska inte ha n�gra parents nu. L�s �nd� for
                // safety! (y)
                char* transformName = new char[transformNameSize];
                ifs.read((char*)parentName, sizeof(char) * parentNameSize);
                ifs.read((char*)transformName, sizeof(char) * transformNameSize);
                // Convert to string. not sure how char* works properly with map if I delete them
                std::string t_transformName(transformName);

                // Delete the char*
                delete parentName;
                delete transformName;

                // Fetch the data we are interested in
                TransformInformation transformDataTemp;
                XMFLOAT3 t_eulerAngles;
                // L�s datan och spara ner i min transforminformationsstruct
                ifs.read((char*)&transformDataTemp.position, sizeof(float) * 3);
                ifs.read((char*)&transformDataTemp.quaternion, sizeof(float) * 4);
                // TODOLH ta bort eulers
                ifs.read((char*)&t_eulerAngles, sizeof(float) * 3);
                ifs.read((char*)&transformDataTemp.scale, sizeof(float) * 3);

                // Skapa de n�dv�ndiga vectorer f�r att bygga upp en tranformationsmatris
                XMVECTOR t_translation = XMLoadFloat3(&transformDataTemp.position);
                XMVECTOR t_quaternion = XMLoadFloat4(&transformDataTemp.quaternion);
                XMVECTOR t_scale = XMLoadFloat3(&transformDataTemp.scale);
                XMVECTOR t_zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

                // SPara ner transformationsmatrisen i en XMFLOAT4x4
                XMFLOAT4X4 t_transformationMatrix;
                XMStoreFloat4x4(&t_transformationMatrix, XMMatrixAffineTransformation(t_scale, t_zero, t_quaternion, t_translation));
                // Push the name to the map with the value of the matrix (XMFLAOT4x4)
                t_transformMap[t_transformName] = t_transformationMatrix;
            }
            return t_transformMap;
        }

        vector<DoremiEngine::Graphic::SkeletalVertex>
        LevelLoaderClient::LoadSkeletalMesh(std::ifstream& p_ifs, const int& p_nrOfMeshes, const std::map<std::string, XMFLOAT4X4>& p_transformMap)
        {
            vector<DoremiEngine::Graphic::SkeletalVertex> t_skeletalVertexBuffer;
            for(int i = 0; i < p_nrOfMeshes; i++)
            {
                // L�s datan och spara ner p� r�tt st�lle. Mesharna har mycket att l�sa kommenterar inte mer �n detta.
                int meshNameSize;
                int transformNameSize;
                int materialNameSize;

                p_ifs.read((char*)&meshNameSize, sizeof(int));
                char* meshName = new char[meshNameSize];
                p_ifs.read((char*)meshName, sizeof(char) * meshNameSize);
                std::string t_meshName(meshName);

                p_ifs.read((char*)&transformNameSize, sizeof(int));
                char* transformName = new char[transformNameSize]; // denna meshens transforms, denna mesh ska anv�nda dess transform v�rden!
                p_ifs.read((char*)transformName, sizeof(char) * transformNameSize);

                std::string t_transformName(transformName);

                p_ifs.read((char*)&materialNameSize, sizeof(int));
                char* materialName = new char[materialNameSize];
                p_ifs.read((char*)materialName, sizeof(char) * materialNameSize);


                int meshID;
                p_ifs.read((char*)&meshID, sizeof(int)); // nog inget som anv�nds atm
                DoremiEditor::Core::MeshData meshData;

                p_ifs.read((char*)&meshData.vertCount, sizeof(int));
                p_ifs.read((char*)&meshData.normalCount, sizeof(int));
                p_ifs.read((char*)&meshData.UVCount, sizeof(int));
                p_ifs.read((char*)&meshData.indCount, sizeof(int));
                p_ifs.read((char*)&meshData.triCount, sizeof(int));

                meshData.positions = new XMFLOAT3[meshData.vertCount];
                meshData.normals = new XMFLOAT3[meshData.normalCount];
                meshData.uvs = new XMFLOAT2[meshData.UVCount];

                meshData.indexPositions = new int[meshData.indCount];
                meshData.indexNormals = new int[meshData.indCount];
                meshData.indexUVs = new int[meshData.indCount];
                meshData.trianglesPerFace = new int[meshData.triCount];
                vector<XMFLOAT3> poss;
                p_ifs.read((char*)meshData.positions, sizeof(XMFLOAT3) * meshData.vertCount);
                p_ifs.read((char*)meshData.normals, sizeof(XMFLOAT3) * meshData.normalCount);
                p_ifs.read((char*)meshData.uvs, sizeof(XMFLOAT2) * meshData.UVCount);

                p_ifs.read((char*)meshData.indexPositions, sizeof(int) * meshData.indCount);
                p_ifs.read((char*)meshData.indexNormals, sizeof(int) * meshData.indCount);
                p_ifs.read((char*)meshData.indexUVs, sizeof(int) * meshData.indCount);
                p_ifs.read((char*)meshData.trianglesPerFace, sizeof(int) * meshData.triCount);
                // Spara ner jointID. Detta skickas till shadern s� att vi kan h�lla s�r p� v�ra olika meshes fast de blendas till en mesh
                int jointID;
                p_ifs.read((char*)&jointID, sizeof(int)); // h�r kommer jointID som ska vara f�r alla denna meshens vertiser, kom p� att jag inte

                // Detta ska bort!! TODOLH
                XMFLOAT4 t_jointOrientation;
                p_ifs.read((char*)&t_jointOrientation, sizeof(float) * 4);

                // H�mta transformationsMatrisen vi sparade undan tidigare genom att plocka ur mappen med hj�lp av namnet vi l�ste h�gre upp i
                // denna forloop
                std::map<std::string, XMFLOAT4X4>::const_iterator t_iterator = p_transformMap.find(t_transformName);
                if(t_iterator == p_transformMap.end())
                {
                    std::cout << "Something went wrong in skeletalanimation loader!" << std::endl;
                }
                XMFLOAT4X4 t_transformationMatrix = p_transformMap.find(t_transformName)->second;

                // Compute vertexdata for graphics, discard data for physics. Det h�r �r n�got magiskt jag tog fr�n den andra inladdningen av
                // vanlig karakt�r
                DirectX::XMFLOAT3 scale = {1.0f, 1.0f, 1.0f}; // TODOXX Should the scale for the player always be one?
                std::vector<DirectX::XMFLOAT3> positionPX;
                std::vector<int> indexPX;
                vector<DoremiEngine::Graphic::Vertex>& vertexBuffer = ComputeVertexAndPositionAndIndex(meshData, scale, positionPX, indexPX);
                size_t length = vertexBuffer.size();
                // Omvandla vertexbuffern till skeletalvertexbuffern
                for(size_t j = 0; j < length; j++)
                {
                    // V�nd p� z (g�rs redan i computevertexandpositionandindex men beh�vs tydligen igen f�r att karakt�ren ska facea r�tt h�ll.
                    vertexBuffer[j].position.z *= -1;
                    // Flytta vertiserna fr�n localspace till Jointspace. Detta g�r s� att v�r "mesh" sitter fast p� jointen d�r den ska. Annars
                    // hade meshen suttit med origo mitt i jointen.
                    XMVECTOR t_vertexPosition = XMLoadFloat3(&vertexBuffer[j].position);
                    XMMATRIX t_transformMatrix = XMLoadFloat4x4(&t_transformationMatrix);
                    // Transforma en positionsvector med transformationsmatrisen vi l�ste in tidigare
                    t_vertexPosition = XMVector3Transform(t_vertexPosition, t_transformMatrix);
                    // SKapa en skeletalvertex. Den enda skillnaden �r att vi har en jointID i skeletalvertexx FInns det ett b�ttre s�tt att g�ra
                    // detta p�?
                    DoremiEngine::Graphic::SkeletalVertex t_vertex;
                    XMStoreFloat3(&t_vertex.position, t_vertexPosition);
                    t_vertex.normal = vertexBuffer[j].normal;
                    t_vertex.textureCoordinate = vertexBuffer[j].textureCoordinate;
                    t_vertex.jointID = jointID;
                    t_skeletalVertexBuffer.push_back(t_vertex);
                }
                // Deletea char*
                delete transformName;
                delete meshName;
                delete materialName;
            }
            return t_skeletalVertexBuffer;
        }

        CharacterDataNames LevelLoaderClient::LoadCharacter(const std::string& p_fileName)
        {
            using namespace std;
            const string fileName = m_sharedContext.GetWorkingDirectory() + p_fileName;
            ifstream ifs;
            ifs.open(fileName, ifstream::in | ofstream::binary);
            if(ifs.is_open() == true)
            {
                // testa o l�sa lite

                // scene name
                int sceneNameSize;
                ifs.read((char*)&sceneNameSize, sizeof(int));
                char* sceneName = new char[sceneNameSize];
                ifs.read((char*)sceneName, sizeof(char) * sceneNameSize);

                // how much different stuff there is
                int nrMats, nrTransforms, nrMeshes, nrLights;
                ifs.read((char*)&nrMats, sizeof(int));
                ifs.read((char*)&nrTransforms, sizeof(int));
                ifs.read((char*)&nrMeshes, sizeof(int));
                ifs.read((char*)&nrLights, sizeof(int));

                // LoadMaterialCharacter(ifs, nrMats);
                LoadMaterial(ifs, nrMats); // TODOKO THis should be instead of the ugly shit above
                // LoadTransformsCharacter(ifs, nrTransforms);
                LoadTransforms(ifs, nrTransforms); // TODOKO DOnt know... This or other shit when character is done...
                LoadMeshes(ifs, nrMeshes);
                LoadLights(ifs, nrLights);
                // BuildEntities();

                // Player specific
                DoremiEngine::Graphic::MeshManager& meshManager = m_sharedContext.GetGraphicModule().GetSubModuleManager().GetMeshManager();
                const std::string meshName = m_meshCoupling[0].meshName;
                std::string textureName;
                textureName = m_materials[m_meshCoupling[0].materialName].diffuseTextureName;

                // Compute vertexdata for graphics, discard data for physics.
                DirectX::XMFLOAT3 scale = {1.0f, 1.0f, 1.0f}; // TODOXX Should the scale for the player always be one?
                std::vector<DirectX::XMFLOAT3> positionPX;
                std::vector<int> indexPX;
                vector<DoremiEngine::Graphic::Vertex>& vertexBuffer = ComputeVertexAndPositionAndIndex(m_meshes[meshName], scale, positionPX, indexPX);

                meshManager.BuildMeshInfoFromBuffer(vertexBuffer, meshName);
                meshManager.BuildMaterialInfo(m_materials[m_meshCoupling[0].materialName]);

                CharacterDataNames o_charData;
                o_charData.meshName = meshName;
                o_charData.materialName = textureName;
                return o_charData;
            }
            else
            {
                throw std::runtime_error("Could not load character.");
            }
        }

        void LevelLoaderClient::LoadFileInternal(const std::string& p_fileName) {}


        bool LevelLoaderClient::BuildComponents(int p_entityId, int p_meshCouplingID, std::vector<DoremiEngine::Graphic::Vertex>& p_vertexBuffer)
        {
            bool r_shouldBuildPhysics = true;

            const ObjectCouplingInfo& meshCoupling = m_meshCoupling[p_meshCouplingID];

            EntityHandler::GetInstance().AddComponent(p_entityId, (int)ComponentType::Transform);
            TransformComponent* transComp = EntityHandler::GetInstance().GetComponentFromStorage<TransformComponent>(p_entityId);

            transComp->position = m_transforms[meshCoupling.transformName].translation;
            transComp->rotation = m_transforms[meshCoupling.transformName].rotation;
            transComp->scale = m_transforms[meshCoupling.transformName].scale;

            DoremiEditor::Core::TransformData transformationData = m_transforms[meshCoupling.transformName];

            // If render, create grapic properties
            if(transformationData.attributes.isRendered)
            {
                EntityHandler::GetInstance().AddComponent(p_entityId, (int)ComponentType::Render);

                RenderComponent* renderComp = EntityHandler::GetInstance().GetComponentFromStorage<RenderComponent>(p_entityId);

                DoremiEngine::Graphic::MeshManager& meshManager = m_sharedContext.GetGraphicModule().GetSubModuleManager().GetMeshManager();
                renderComp->mesh = meshManager.BuildMeshInfoFromBuffer(p_vertexBuffer, meshCoupling.meshName);

                std::string textureName = m_materials[meshCoupling.materialName].diffuseTextureName;

                renderComp->material = meshManager.BuildMaterialInfo(m_materials[meshCoupling.materialName]);
            }

            // if frequency platform
            if(transformationData.attributes.frequencyAffected)
            {
                r_shouldBuildPhysics = false;
                EntityHandler::GetInstance().AddComponent(p_entityId, static_cast<uint32_t>(ComponentType::NetworkObject) |
                                                                          static_cast<uint32_t>(ComponentType::RigidBody) |
                                                                          static_cast<uint32_t>(ComponentType::PhysicalMaterial));


                // Rigid body comp
                RigidBodyComponent* t_rigidBodyComp = GetComponent<RigidBodyComponent>(p_entityId);

                // t_rigidBodyComp->boxDims = dimension;
                t_rigidBodyComp->flags =
                    static_cast<RigidBodyFlags>(static_cast<uint32_t>(RigidBodyFlags::kinematic) | static_cast<uint32_t>(RigidBodyFlags::drain));
                t_rigidBodyComp->geometry = RigidBodyGeometry::dynamicBox;
            }

            // If non physic object
            if(transformationData.attributes.isSpawner || transformationData.attributes.spawnPointID > -1 ||
               transformationData.attributes.startOrEndPoint == 2 || transformationData.attributes.checkPointID > -1 || transformationData.attributes.isDangerous)
            {
                r_shouldBuildPhysics = false;
            }


            // If we're rigid body
            if(EntityHandler::GetInstance().HasComponents(p_entityId, static_cast<uint32_t>(ComponentType::RigidBody)))
            {
                // Get us our rigid body manager
                DoremiEngine::Physics::RigidBodyManager& rigidBodyManager = m_sharedContext.GetPhysicsModule().GetRigidBodyManager();

                // calulate aab
                XMFLOAT3 centerPoint, minPoint, maxPoint;
                CalculateAABBBoundingBox(p_vertexBuffer, transformationData, maxPoint, minPoint, centerPoint);

                XMFLOAT3 dimension = XMFLOAT3(abs(minPoint.x - maxPoint.x) / 2.0f, abs(minPoint.y - maxPoint.y) / 2.0f, abs(minPoint.z - maxPoint.z) / 2.0f);
                RigidBodyComponent* bodyComp = GetComponent<RigidBodyComponent>(p_entityId);
                bodyComp->boxDims = dimension;

                int materialTriggID = m_sharedContext.GetPhysicsModule().GetPhysicsMaterialManager().CreateMaterial(0.0f, 0.0f, 0.0f);

                // Get the material. This is haxxy. It probably works most of the time
                PhysicsMaterialComponent* matComp = GetComponent<PhysicsMaterialComponent>(p_entityId);
                matComp->p_materialID = materialTriggID;

                switch(bodyComp->geometry)
                {
                    case RigidBodyGeometry::dynamicBox:
                        rigidBodyManager.AddBoxBodyDynamic(p_entityId, transComp->position, XMFLOAT4(0, 0, 0, 1), bodyComp->boxDims, matComp->p_materialID);
                        break;
                    case RigidBodyGeometry::dynamicSphere:
                        rigidBodyManager.AddSphereBodyDynamic(p_entityId, transComp->position, bodyComp->radius);
                        break;
                    case RigidBodyGeometry::dynamicCapsule:
                        rigidBodyManager.AddCapsuleBodyDynamic(p_entityId, transComp->position, XMFLOAT4(0, 0, 0, 1), bodyComp->height, bodyComp->radius);
                        break;
                    case RigidBodyGeometry::staticBox:
                        rigidBodyManager.AddBoxBodyStatic(p_entityId, transComp->position, XMFLOAT4(0, 0, 0, 1), bodyComp->boxDims, matComp->p_materialID);
                        break;
                    default:
                        break;
                }
                // Apply flags
                if(((int)bodyComp->flags & (int)RigidBodyFlags::kinematic) == (int)RigidBodyFlags::kinematic)
                {
                    rigidBodyManager.SetKinematicActor(p_entityId, true);

                    // TODOJB TODOXX haxy callback filtering. Read comment in LevelLoader::SetPhysicalAttributesOnMesh
                    rigidBodyManager.SetCallbackFiltering(p_entityId, 3, 0, 0, 2);
                }
                if(((int)bodyComp->flags & (int)RigidBodyFlags::trigger) == (int)RigidBodyFlags::trigger)
                {
                    rigidBodyManager.SetTrigger(p_entityId, true);
                }
                if(((int)bodyComp->flags & (int)RigidBodyFlags::drain) == (int)RigidBodyFlags::drain)
                {
                    rigidBodyManager.SetDrain(p_entityId, true);
                }
                if(((int)bodyComp->flags & (int)RigidBodyFlags::ignoredDEBUG) == (int)RigidBodyFlags::ignoredDEBUG)
                {
                    rigidBodyManager.SetIgnoredDEBUG(p_entityId);
                }
            }


            return r_shouldBuildPhysics;
        }

        void LevelLoaderClient::BuildLights()
        {
            DoremiEngine::Graphic::LightManager& lightManager = m_sharedContext.GetGraphicModule().GetSubModuleManager().GetLightManager();
            int i = 0;
            for(auto& light : m_lights)
            {
                lightManager.AddLight(light.type, light.intensity, light.colorDiffuse, light.coneAngle, light.direction, light.penumAgle,
                                      m_transforms[m_lightNames[i].first].translation);
                ++i;
            }
            lightManager.UpdateLights();
        }
    }
}
