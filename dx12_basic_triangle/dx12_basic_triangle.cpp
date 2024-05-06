
// dx12_basic_triangle.cpp
// �O�p�`�̃|���S������邾���̃T���v��

#include "./dx12_basic_triangle.h"

#include <windows.h>
#include <cassert>
#include <fstream>
#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace {
    // �o�C�i���t�@�C���̓ǂݍ��݁B�V�F�[�_�o�C�i���p
    HRESULT ReadDataFromFile(const wchar_t* filename, std::vector<char>& data)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        }

        size_t fileSize = (size_t)file.tellg();
        data.resize(fileSize);

        file.seekg(0);
        file.read(data.data(), fileSize);
        file.close();

        return S_OK;
    }

    // �V�F�[�_�o�C�i���̃��[�h
    HRESULT LoadShader(const wchar_t* filename, ID3DBlob** shaderBlob)
    {
        std::vector<char> shaderData;
        HRESULT hr = ReadDataFromFile(filename, shaderData);
        assert(hr == S_OK);

        hr = D3DCreateBlob(shaderData.size(), shaderBlob);
        assert(hr == S_OK);

        std::copy(shaderData.begin(), shaderData.end(), reinterpret_cast<char*>((*shaderBlob)->GetBufferPointer()));

        return S_OK;
    }
}

// �A�v���P�[�V�����̏������B�N������1�x�����Ă�
void Dx12BasicTriangle::init(HWND hWnd)
{
    // DirectX 12�̏�����
    initDirectX12();

    // �R�}���h�L���[�̍쐬
    initCommandQueue();

    // �X���b�v�`�F�C���̍쐬
    initSwapChain(hWnd);

    // �t�F���X�̍쐬
    initFence();

    // ���_�o�b�t�@�̍쐬
    initVertexBuffer();

    // �V�F�[�_�̍쐬
    initShaders();

    // �p�C�v���C���X�e�[�g�̍쐬
    initPipelineState();

    // �r���[�|�[�g�̐ݒ�
    m_viewport.TopLeftX = 0.0f;
    m_viewport.TopLeftY = 0.0f;
    m_viewport.Width = kRenderWidth;
    m_viewport.Height = kRenderHeight;
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;

    // �V�U�[�̐ݒ�
    m_scissorRect.left = 0;
    m_scissorRect.top = 0;
    m_scissorRect.right = kRenderWidth;
    m_scissorRect.bottom = kRenderHeight;

    // �v���W�F�N�V�����s��
    constexpr float aspectRatio = static_cast<float>(kRenderWidth) / kRenderHeight;
    m_proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(45.0f), aspectRatio, 0.1f, 100.0f);

    // �J�����͌��_�Œ�Ƃ��Ă�̂ŎO�p�`��+Z�����ɏ����������ꏊ�ɒu��
    m_triangleTrans = DirectX::XMVectorSet(0.0f, 0.0f, 2.5f, 1.0f);
}

// DirectX 12�̏�����
void Dx12BasicTriangle::initDirectX12()
{
    HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&m_dxgiFactory));
    assert(hr == S_OK);

    // ����͍ŏ��Ɍ������f�B�X�v���C�A�_�v�^���g�������B�{���͕�����������X�y�b�N�����Č��߂��肵�Ȃ��ƂȂ�Ȃ�
    IDXGIAdapter1* adapter = nullptr;
    hr = m_dxgiFactory->EnumAdapters1(0, &adapter);
    assert(hr == S_OK);

    hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device));
    assert(hr == S_OK);
}

// �R�}���h�L���[�̍쐬
void Dx12BasicTriangle::initCommandQueue()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    HRESULT hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
    assert(hr == S_OK);

    hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
    assert(hr == S_OK);

    hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator, nullptr, IID_PPV_ARGS(&m_commandList));
    assert(hr == S_OK);

    hr = m_commandList->Close();
    assert(hr == S_OK);
}

// �X���b�v�`�F�C���̍쐬
void Dx12BasicTriangle::initSwapChain(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = kBufferCount;
    swapChainDesc.Width = kRenderWidth;
    swapChainDesc.Height = kRenderHeight;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;

    HRESULT hr = m_dxgiFactory->CreateSwapChainForHwnd(
        m_commandQueue,
        hWnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        (IDXGISwapChain1**)&m_swapChain
    );
    assert(hr == S_OK);

    // �X���b�v�`�F�C�����烌���_�[�^�[�Q�b�g���擾
    for (int i = 0; i < kBufferCount; ++i)
    {
        hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
        assert(hr == S_OK);
    }

    // �����_�[�^�[�Q�b�g�r���[�̍쐬
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = kBufferCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
    assert(hr == S_OK);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    for (UINT i = 0; i < kBufferCount; i++)
    {
        m_device->CreateRenderTargetView(m_renderTargets[i], nullptr, rtvHandle);
        rtvHandle.ptr += m_rtvDescriptorSize;
    }
}

// �t�F���X�̍쐬
void Dx12BasicTriangle::initFence()
{
    HRESULT hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
    assert(hr == S_OK);

    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, L"Flip complete");
    assert(m_fenceEvent != NULL);
}

// ���_�o�b�t�@�̍쐬
void Dx12BasicTriangle::initVertexBuffer()
{
    // ���_�f�[�^�̒�`
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 color;
    };

    constexpr float kHeight = 0.5f * 1.7320508f;
    Vertex vertices[] =
    {
        //   x      y               z       r     g     b
        { { 0.0f, -0.5f + kHeight, 0.0f}, {1.0f, 0.0f, 0.0f} }, // ��
        { { 0.5f, -0.5f,           0.0f}, {0.0f, 1.0f, 0.0f} }, // �E��
        { {-0.5f, -0.5f,           0.0f}, {0.0f, 0.0f, 1.0f} }, // ����
    };

    const UINT bufferSize = sizeof(vertices);

    // ���_�o�b�t�@�p�̃��\�[�X�쐬
    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = bufferSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    HRESULT hr = m_device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_vertexBuffer)
    );
    assert(hr == S_OK);

    // ���_�f�[�^���R�s�[
    void* pVertexDataBegin;
    hr = m_vertexBuffer->Map(0, nullptr, &pVertexDataBegin);
    assert(hr == S_OK);

    memcpy(pVertexDataBegin, vertices, bufferSize);

    m_vertexBuffer->Unmap(0, nullptr);

    // ���_�o�b�t�@�r���[�쐬
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);
    m_vertexBufferView.SizeInBytes = bufferSize;
}

// �V�F�[�_�̍쐬
void Dx12BasicTriangle::initShaders()
{
    // ���[�g�p�����[�^�̐ݒ�B���_�V�F�[�_�p�ɒ萔�o�b�t�@��1����悤�ɂ���
    D3D12_ROOT_PARAMETER rootParameters[1];
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].Descriptor.RegisterSpace = 0;
    rootParameters[0].Descriptor.ShaderRegister = 0;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    // �p�����[�^1�̃��[�g�V�O�l�`���ɐݒ�
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 1;
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // ���[�g�V�O�l�`���̃V���A���C�Y
    ID3DBlob* signature;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
    assert(hr == S_OK);

    // ���[�g�V�O�l�`���̍쐬
    hr = m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
    assert(hr == S_OK);
    signature->Release();

    // �萔�o�b�t�@�p���\�[�X�̍쐬
    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

    UINT bufferSize = sizeof(DirectX::XMFLOAT4X4);

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = bufferSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    hr = m_device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_constantBuffer));
    assert(hr == S_OK);

    // �V�F�[�_�o�C�i�����t�@�C������ǂݍ���
#ifdef _DEBUG
    constexpr wchar_t vertexShaderName[] = L"VertexShader_debug.cso";
    constexpr wchar_t pixelShaderName[]  = L"PixelShader_debug.cso";
#else
    constexpr wchar_t vertexShaderName[] = L"VertexShader_release.cso";
    constexpr wchar_t pixelShaderName[] = L"PixelShader_release.cso";
#endif

    hr = LoadShader(vertexShaderName, &m_vertexShaderBlob);
    assert(hr == S_OK);

    hr = LoadShader(pixelShaderName, &m_pixelShaderBlob);
    assert(hr == S_OK);
}

// �p�C�v���C���X�e�[�g�̍쐬
void Dx12BasicTriangle::initPipelineState()
{
    // �p�C�v���C���X�e�[�g�I�u�W�F�N�g�̐ݒ�
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = m_rootSignature;
    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;

    // �V�F�[�_�̐ݒ�
    {
        D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
        vertexShaderBytecode.pShaderBytecode = m_vertexShaderBlob->GetBufferPointer();
        vertexShaderBytecode.BytecodeLength = m_vertexShaderBlob->GetBufferSize();
        psoDesc.VS = vertexShaderBytecode;

        D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
        pixelShaderBytecode.pShaderBytecode = m_pixelShaderBlob->GetBufferPointer();
        pixelShaderBytecode.BytecodeLength = m_pixelShaderBlob->GetBufferSize();
        psoDesc.PS = pixelShaderBytecode;
    }

    {
        // ���̓��C�A�E�g�̒�`
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    }

    {
        // ���X�^���C�U�[�X�e�[�g�̐ݒ�
        D3D12_RASTERIZER_DESC rasterizerDesc = {};
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
        rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rasterizerDesc.DepthClipEnable = FALSE;
        rasterizerDesc.MultisampleEnable = FALSE;
        rasterizerDesc.AntialiasedLineEnable = FALSE;
        rasterizerDesc.ForcedSampleCount = 0;
        rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        psoDesc.RasterizerState = rasterizerDesc;
    }

    {
        // �u�����h�X�e�[�g�̐ݒ�
        D3D12_BLEND_DESC blendDesc = {};
        blendDesc.AlphaToCoverageEnable = FALSE;
        blendDesc.IndependentBlendEnable = FALSE;
        D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc = {};
        defaultRenderTargetBlendDesc.BlendEnable = FALSE;
        defaultRenderTargetBlendDesc.LogicOpEnable = FALSE;
        defaultRenderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        {
            blendDesc.RenderTarget[i] = defaultRenderTargetBlendDesc;
        }

        psoDesc.BlendState = blendDesc;
    }

    {
        // �f�v�X�X�e���V���X�e�[�g�̐ݒ�
        D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
        depthStencilDesc.DepthEnable = FALSE;
        depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.StencilEnable = FALSE;

        psoDesc.DepthStencilState = depthStencilDesc;
    }

    // �p�C�v���C���X�e�[�g�I�u�W�F�N�g�̍쐬
    HRESULT hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
    assert(hr == S_OK);
}

// �V�[���̍X�V����
void Dx12BasicTriangle::update(UINT64 frameNumber, float deltaTime)
{
    // 4�b��1��]����悤�ɉ�
    m_triangleRot = DirectX::XMQuaternionMultiply(
        DirectX::XMQuaternionRotationRollPitchYaw(0.0f, 0.5f * DirectX::XM_PI * deltaTime, 0.0f),
        m_triangleRot);
}

// �V�[���̕`�揈��
void Dx12BasicTriangle::draw(UINT64 frameNumber)
{
    // ���̃t���[���̕`��Ƀ_�u���o�b�t�@�̂ǂ�����g�p���邩�̃C���f�b�N�X
    auto bufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    // �����_�[�^�[�Q�b�g�r���[�̐ݒ�
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += bufferIndex * m_rtvDescriptorSize;

    // �R�}���h�A���P�[�^�����Z�b�g
    HRESULT hr = m_commandAllocator->Reset();
    assert(hr == S_OK);

    // �R�}���h���X�g�����Z�b�g
    hr = m_commandList->Reset(m_commandAllocator, nullptr);
    assert(hr == S_OK);

    // �����_�[�^�[�Q�b�g���g�p�\�ɂ���o���A
    D3D12_RESOURCE_BARRIER barrierPresentToRt = {};
    barrierPresentToRt.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrierPresentToRt.Transition.pResource = m_renderTargets[bufferIndex];
    barrierPresentToRt.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrierPresentToRt.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    m_commandList->ResourceBarrier(1, &barrierPresentToRt);

    // �����_�[�^�[�Q�b�g��ݒ�
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // �N���A�J���[�œh��Ԃ�
    constexpr float clearColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // �r���[�|�[�g�ƃV�U�[��ݒ�
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // �p�C�v���C���X�e�[�g��ݒ�
    m_commandList->SetPipelineState(m_pipelineState);

    // �萔�o�b�t�@��ݒ�
    m_commandList->SetGraphicsRootSignature(m_rootSignature);
    m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress());

    {
        DirectX::XMMATRIX rot = DirectX::XMMatrixRotationQuaternion(m_triangleRot);
        DirectX::XMMATRIX trans = DirectX::XMMatrixTranslationFromVector(m_triangleTrans);

        DirectX::XMFLOAT4X4 objToProj;
        DirectX::XMStoreFloat4x4(&objToProj, DirectX::XMMatrixTranspose(rot * trans * m_proj));

        UINT8* pConstantBufferBegin;
        HRESULT hr = m_constantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pConstantBufferBegin));
        assert(hr == S_OK);

        memcpy(pConstantBufferBegin, &objToProj, sizeof(objToProj));

        m_constantBuffer->Unmap(0, nullptr);
    }

    // �`�悷��`��͎O�p�`���X�g
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // ���_�o�b�t�@��ݒ�
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    // 3���_��`��
    m_commandList->DrawInstanced(3, 1, 0, 0);

    // �����_�[�^�[�Q�b�g���g�p�s�ɂ���o���A
    D3D12_RESOURCE_BARRIER barrierRtToPresent = {};
    barrierRtToPresent.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrierRtToPresent.Transition.pResource = m_renderTargets[bufferIndex];
    barrierRtToPresent.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrierRtToPresent.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    m_commandList->ResourceBarrier(1, &barrierRtToPresent);

    // �R�}���h���X�g�I��
    hr = m_commandList->Close();
    assert(hr == S_OK);

    // �R�}���h���X�g��GPU�ɑ���
    ID3D12CommandList* ppCommandLists[] = { m_commandList };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // �t���b�v����
    hr = m_swapChain->Present(1, 0);
    assert(hr == S_OK);

    // �t�F���X�փV�O�i���𑗂�R�}���h��ς�
    m_commandQueue->Signal(m_fence, frameNumber);

    // �V�O�i��������܂ő҂�
    if (m_fence->GetCompletedValue() < frameNumber)
    {
        HRESULT hr = m_fence->SetEventOnCompletion(frameNumber, m_fenceEvent);
        assert(hr == S_OK);

        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

// �A�v���P�[�V�����̏I������
void Dx12BasicTriangle::finalize()
{
    auto safeRelease = [](auto& p) { if (p != nullptr) { p->Release(); p = nullptr; } };

    safeRelease(m_pipelineState);

    safeRelease(m_vertexShaderBlob);
    safeRelease(m_pixelShaderBlob);
    safeRelease(m_constantBuffer);
    safeRelease(m_rootSignature);

    safeRelease(m_vertexBuffer);

    CloseHandle(m_fenceEvent);
    safeRelease(m_fence);

    safeRelease(m_rtvHeap);
    for (int i = 0; i < kBufferCount; ++i)
    {
        safeRelease(m_renderTargets[i]);
    }
    safeRelease(m_swapChain);

    safeRelease(m_commandList);
    safeRelease(m_commandAllocator);
    safeRelease(m_commandQueue);

    safeRelease(m_device);
    safeRelease(m_dxgiFactory);
}
