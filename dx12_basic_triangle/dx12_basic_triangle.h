
// dx12_basic_triangle.h
// 三角形のポリゴンが回るだけのサンプル

#pragma once

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>

// アプリケーション本体
class Dx12BasicTriangle
{
public:
	// レンダリング解像度
	static constexpr int kRenderWidth  = 1280;
	static constexpr int kRenderHeight = 720;

	// スワップチェインが作るバッファの個数
	static constexpr int kBufferCount = 2;

	void init(HWND hWnd);								// アプリケーションの初期化。起動時に1度だけ呼ぶ
	void update(UINT64 frameNumber, float deltaTime);	// シーンの更新処理
	void draw(UINT64 frameNumber);						// シーンの描画処理
	void finalize();									// アプリケーションの終了処理

protected:
	void initDirectX12();				// DirectX 12の初期化
	void initCommandQueue();			// コマンドキューの作成
	void initSwapChain(HWND hWnd);		// スワップチェインの作成
	void initFence();					// フェンスの作成
	void initVertexBuffer();			// 頂点バッファの作成
	void initShaders();					// シェーダの作成
	void initPipelineState();			// パイプラインステートの作成

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