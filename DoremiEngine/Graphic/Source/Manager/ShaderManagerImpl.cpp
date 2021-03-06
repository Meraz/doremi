#pragma once
#include <Internal/Manager/ShaderManagerImpl.hpp>
#include <Internal/Manager/DirectXManagerImpl.hpp>
#include <Internal/Manager/SubModuleManagerImpl.hpp>
#include <GraphicModuleImplementation.hpp>
#include <Internal/Shader/PixelShaderImpl.hpp>
#include <Internal/Shader/VertexShaderImpl.hpp>
#include <Internal/Shader/ComputeShaderImpl.hpp>
#include <Internal/Shader/GeometryShaderImpl.hpp>
#include <Internal/Manager/ComputeShaderManagerImpl.hpp>
#include <GraphicModuleContext.hpp>
#include <string>
#include <HelpFunctions.hpp>

#include <d3dcompiler.h>
#include <d3d11_1.h>
namespace DoremiEngine
{
    namespace Graphic
    {
        ShaderManagerImpl::ShaderManagerImpl(const GraphicModuleContext& p_graphicContext) : m_graphicContext(p_graphicContext) {}
        ShaderManagerImpl::~ShaderManagerImpl() {}
        VertexShader* ShaderManagerImpl::BuildVertexShader(const std::string& p_fileName, D3D11_INPUT_ELEMENT_DESC p_inputDescription[], int p_arraySize)
        {
            std::string filePath = m_graphicContext.m_workingDirectory + "ShaderFiles/" + p_fileName;
            ID3D11VertexShader* shader;
            ID3D11InputLayout* inputLayout;
            DirectXManager& m_directX = m_graphicContext.m_graphicModule->GetSubModuleManager().GetDirectXManager();
            DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
            shaderFlags |= D3DCOMPILE_DEBUG;
            shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
            ID3DBlob* tShader;
            std::wstring convertedName = StringToWstring(filePath);
            HRESULT res = D3DCompileFromFile(convertedName.c_str(), 0, 0, "VS_main", "vs_5_0", shaderFlags, 0, &tShader, 0);

            bool success = CheckHRESULT(res, "Error Compiling from file " + filePath);
            if(!success)
            {
                return nullptr;
            }
            res = m_directX.GetDevice()->CreateVertexShader(tShader->GetBufferPointer(), tShader->GetBufferSize(), NULL, &shader);
            success = CheckHRESULT(res, "Error Creating Vertex Shader");
            if(!success)
            {
                return nullptr;
            }

            res = m_directX.GetDevice()->CreateInputLayout(p_inputDescription, p_arraySize, tShader->GetBufferPointer(), tShader->GetBufferSize(), &inputLayout);
            success = CheckHRESULT(res, "Error setting input layout");
            if(!success)
            {
                return nullptr;
            }

            VertexShader* newShader = new VertexShaderImpl();
            newShader->SetShaderHandle(shader);
            newShader->SetShaderName(p_fileName);
            newShader->SetInputLayout(inputLayout);
            return newShader;
        }

        VertexShader* ShaderManagerImpl::BuildVertexShaderWithoutInput(const std::string& p_fileName)
        {
            std::string filePath = m_graphicContext.m_workingDirectory + "ShaderFiles/" + p_fileName;
            ID3D11VertexShader* shader = nullptr;
            ID3D11InputLayout* inputLayout = nullptr;
            DirectXManager& m_directX = m_graphicContext.m_graphicModule->GetSubModuleManager().GetDirectXManager();
            DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
            shaderFlags |= D3DCOMPILE_DEBUG;
            shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
            ID3DBlob* tShader;
            std::wstring convertedName = StringToWstring(filePath);
            HRESULT res = D3DCompileFromFile(convertedName.c_str(), 0, 0, "VS_main", "vs_5_0", shaderFlags, 0, &tShader, 0);

            bool success = CheckHRESULT(res, "Error Compiling from file " + filePath);
            if (!success)
            {
                return nullptr;
            }
            res = m_directX.GetDevice()->CreateVertexShader(tShader->GetBufferPointer(), tShader->GetBufferSize(), NULL, &shader);
            success = CheckHRESULT(res, "Error Creating Vertex Shader");
            if (!success)
            {
                return nullptr;
            }

            VertexShader* newShader = new VertexShaderImpl();
            newShader->SetShaderHandle(shader);
            newShader->SetShaderName(p_fileName);
            newShader->SetInputLayout(inputLayout);
            return newShader;
        }


        GeometryShader* ShaderManagerImpl::BuildGeometryShader(const std::string& p_fileName)
        {
            // TODOKO make shaders save in map just like mesh info
            std::string filePath = m_graphicContext.m_workingDirectory + "ShaderFiles/" + p_fileName;
            ID3D11GeometryShader* shader;
            DirectXManager& m_directX = m_graphicContext.m_graphicModule->GetSubModuleManager().GetDirectXManager();

            DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
            shaderFlags |= D3DCOMPILE_DEBUG;
            shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
            ID3DBlob* tShader;
            std::wstring convertedName = StringToWstring(filePath);
            HRESULT res = D3DCompileFromFile(convertedName.c_str(), 0, 0, "GS_main", "gs_5_0", shaderFlags, 0, &tShader, 0);
            bool success = CheckHRESULT(res, "Error Compiling from file " + filePath);
            if(!success)
            {
                return nullptr;
            }

            res = m_directX.GetDevice()->CreateGeometryShader(tShader->GetBufferPointer(), tShader->GetBufferSize(), NULL, &shader);
            success = CheckHRESULT(res, "Error Compiling from file " + filePath);
            if(!success)
            {
                return nullptr;
            }
            GeometryShader* newShader = new GeometryShaderImpl();
            newShader->SetShaderHandle(shader);
            newShader->SetShaderName(p_fileName);
            return newShader;
        }

        PixelShader* ShaderManagerImpl::BuildPixelShader(const std::string& p_fileName)
        {
            // TODOKO make shaders save in map just like mesh info
            std::string filePath = m_graphicContext.m_workingDirectory + "ShaderFiles/" + p_fileName;
            ID3D11PixelShader* shader;
            DirectXManager& m_directX = m_graphicContext.m_graphicModule->GetSubModuleManager().GetDirectXManager();
            DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
            shaderFlags |= D3DCOMPILE_DEBUG;
            shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
            ID3DBlob* tShader;
            std::wstring convertedName = StringToWstring(filePath);
            HRESULT res = D3DCompileFromFile(convertedName.c_str(), 0, 0, "PS_main", "ps_5_0", shaderFlags, 0, &tShader, 0);
            bool success = CheckHRESULT(res, "Error Compiling from file " + filePath);
            if(!success)
            {
                return nullptr;
            }

            res = m_directX.GetDevice()->CreatePixelShader(tShader->GetBufferPointer(), tShader->GetBufferSize(), NULL, &shader);
            success = CheckHRESULT(res, "Error Compiling from file " + filePath);
            if(!success)
            {
                return nullptr;
            }

            PixelShader* newShader = new PixelShaderImpl();
            newShader->SetShaderHandle(shader);
            newShader->SetShaderName(p_fileName);
            return newShader;
        }
        ComputeShader* ShaderManagerImpl::BuildComputeShader(const std::string& p_fileName)
        {
            std::string filePath = m_graphicContext.m_workingDirectory + "ShaderFiles/" + p_fileName;
            ID3D11ComputeShader* shader;
            DirectXManager& m_directX = m_graphicContext.m_graphicModule->GetSubModuleManager().GetDirectXManager();
            DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_IEEE_STRICTNESS | D3DCOMPILE_PREFER_FLOW_CONTROL;
#if defined(DEBUG) || defined(_DEBUG)
            shaderFlags |= D3DCOMPILE_DEBUG;
            shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
            ID3DBlob* tShader;
            std::wstring convertedName = StringToWstring(filePath);
            HRESULT res = D3DCompileFromFile(convertedName.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CS_main", "cs_5_0", shaderFlags, 0, &tShader, 0);
            bool success = CheckHRESULT(res, "Error Compiling from file " + filePath);
            if(!success)
            {
                return nullptr;
            }

            res = m_directX.GetDevice()->CreateComputeShader(tShader->GetBufferPointer(), tShader->GetBufferSize(), NULL, &shader);
            success = CheckHRESULT(res, "Error Compiling from file " + filePath);
            if(!success)
            {
                return nullptr;
            }

            m_directX.GetDeviceContext()->CSSetShader(shader, nullptr, 0);

            ComputeShader* newShader = new ComputeShaderImpl();
            newShader->SetShaderHandle(shader);
            newShader->SetShaderName(p_fileName);
            return newShader;
        }

        void ShaderManagerImpl::SetActiveVertexShader(VertexShader* p_shader)
        {
            DirectXManager& m_directX = m_graphicContext.m_graphicModule->GetSubModuleManager().GetDirectXManager();
            m_directX.GetDeviceContext()->VSSetShader(p_shader->GetShaderHandle(), 0, 0);
            m_directX.GetDeviceContext()->IASetInputLayout(p_shader->GetInputLayout());
        }
        void ShaderManagerImpl::SetActivePixelShader(PixelShader* p_shader)
        {
            DirectXManager& m_directX = m_graphicContext.m_graphicModule->GetSubModuleManager().GetDirectXManager();
            m_directX.GetDeviceContext()->PSSetShader(p_shader->GetShaderHandle(), 0, 0);
        }
        void ShaderManagerImpl::SetActiveGeometryShader(GeometryShader* p_shader)
        {
            DirectXManager& m_directX = m_graphicContext.m_graphicModule->GetSubModuleManager().GetDirectXManager();
            m_directX.GetDeviceContext()->GSSetShader(p_shader->GetShaderHandle(), 0, 0);
        }
        void ShaderManagerImpl::SetActiveComputeShader(ComputeShader* p_shader)
        {
            DirectXManager& m_directX = m_graphicContext.m_graphicModule->GetSubModuleManager().GetDirectXManager();
            m_directX.GetDeviceContext()->CSSetShader(p_shader->GetShaderHandle(), 0, 0);
        }
        void ShaderManagerImpl::RemoveGeometryShader()
        {
            DirectXManager& m_directX = m_graphicContext.m_graphicModule->GetSubModuleManager().GetDirectXManager();
            m_directX.GetDeviceContext()->GSSetShader(nullptr, 0, 0);
        }
    }
}