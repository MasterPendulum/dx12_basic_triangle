
// dx12_basic_triangle.h
// �O�p�`�̃|���S������邾���̃T���v��

#pragma once

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>

// �A�v���P�[�V�����{��
class Dx12BasicTriangle
{
public:
	// �����_�����O�𑜓x
	static constexpr int kRenderWidth  = 1280;
	static constexpr int kRenderHeight = 720;

	// �X���b�v�`�F�C�������o�b�t�@�̌�
	static constexpr int kBufferCount = 2;

	void init(HWND hWnd);								// �A�v���P�[�V�����̏������B�N������1�x�����Ă�
	void update(UINT64 frameNumber, float deltaTime);	// �V�[���̍X�V����
	void draw(UINT64 frameNumber);						// �V�[���̕`�揈��
	void finalize();									// �A�v���P�[�V�����̏I������

protected:
	void initDirectX12();				// DirectX 12�̏�����
	void initCommandQueue();			// �R�}���h�L���[�̍쐬
	void initSwapChain(HWND hWnd);		// �X���b�v�`�F�C���̍쐬
	void initFence();					// �t�F���X�̍쐬
	void initVertexBuffer();			// ���_�o�b�t�@�̍쐬
	void initShaders();					// �V�F�[�_�̍쐬
	void initPipelineState();			// �p�C�v���C���X�e�[�g�̍쐬

private:
	IDXGIFactory6*				m_dxgiFactory		= nullptr;
	ID3D12Device*				m_device			= nullptr;

	ID3D12CommandQueue*			m_commandQueue		= nullptr;
	ID3D12CommandAllocator*		m_commandAllocator	= nullptr;
	ID3D12GraphicsCommandList*	m_commandList		= nullptr;

	IDXGISwapChain4*			m_swapChain						= nullptr;
	ID3D12Resource*				m_renderTargets[kBufferCount]	= {};
	ID3D12DescriptorHeap*		m_rtvHeap						= nullptr;
	UINT						m_rtvDescriptorSize				= 0;

	ID3D12Fence*				m_fence				= nullptr;
	HANDLE						m_fenceEvent		= NULL;

	ID3D12Resource*				m_vertexBuffer		= nullptr;
	D3D12_VERTEX_BUFFER_VIEW	m_vertexBufferView	= {};

	ID3D12RootSignature*		m_rootSignature			= nullptr;
	ID3D12Resource*				m_constantBuffer		= nullptr;
	ID3DBlob*					m_vertexShaderBlob		= nullptr;
	ID3DBlob*					m_pixelShaderBlob		= nullptr;

	ID3D12PipelineState*		m_pipelineState		= nullptr;

	D3D12_VIEWPORT				m_viewport			= {};
	D3D12_RECT					m_scissorRect		= {};

	DirectX::XMMATRIX			m_proj				= DirectX::XMMatrixIdentity();
	DirectX::XMVECTOR			m_triangleTrans		= DirectX::XMVectorZero();
	DirectX::XMVECTOR			m_triangleRot		= DirectX::XMQuaternionIdentity();
};