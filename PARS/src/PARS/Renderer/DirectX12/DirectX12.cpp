#include "stdafx.h"
#include "PARS/Core/Window.h"
#include "PARS/Renderer/DirectX12/DirectX12.h"

namespace PARS
{
	DirectX12::DirectX12(const WindowInfo& info)
		: m_WindowInfo(info)
	{
	}

	DirectX12::~DirectX12()
	{
	}
	
	bool DirectX12::Initailize()
	{
		bool result = CreateDevice();
		if (!result) return false;

		result = CreateCommandObjects();
		if (!result) return false;

		CreateSwapChain();

		result = CreateRtvDsvHeap();
		if (!result) return false;

		result = CreateRenderTargetViews();
		if (!result) return false;

		CreateDepthStecilView();

		SetViewAndScissor();
		
		return true;
	}

	void DirectX12::ShutDown()
	{
		WaitForGpuCompelete();

		CloseHandle(m_FenceEvent);

		if(m_DepthStencilBuffer!=nullptr) m_DepthStencilBuffer->Release();
		if (m_DsvDescriptorHeap != nullptr) m_DsvDescriptorHeap->Release();
		for (auto rtvBuffer : m_RenderTargetBuffers)
		{
			if (rtvBuffer != nullptr) rtvBuffer->Release();
		}
		if (m_RtvDescriptorHeap != nullptr) m_RtvDescriptorHeap->Release();
		if (m_SwapChain != nullptr) m_SwapChain->Release();
		if (m_CommandList != nullptr) m_CommandList->Release();
		if (m_CommandAllocator != nullptr) m_CommandAllocator->Release();
		if (m_CommandQueue != nullptr) m_CommandQueue->Release();
		if (m_Fence != nullptr) m_Fence->Release();
		if (m_Device != nullptr) m_Device->Release();
		if (m_Factory != nullptr) m_Factory->Release();

#if defined(_DEBUG)
		IDXGIDebug1* giDebug = nullptr;
		DXGIGetDebugInterface1(0, IID_PPV_ARGS(&giDebug));
		HRESULT result = giDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
		giDebug->Release();
#endif

	}

	bool DirectX12::CreateDevice()
	{
		HRESULT result;
		UINT factoryFlags = 0;


#if defined(_DEBUG)
		//Enable D3D12 debug layer.
		{
			ID3D12Debug* debugController;
			result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
			if (FAILED(result)) return false;
			debugController->EnableDebugLayer();
			debugController->Release();
			factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
#endif

		//Create Factory
		result = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_Factory));
		if (FAILED(result)) return false;

		//Try to create hardware device
		IDXGIAdapter1* adapter = nullptr;
		for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_Factory->EnumAdapters1(i, &adapter); ++i)
		{
			DXGI_ADAPTER_DESC1 adapterDesc;
			adapter->GetDesc1(&adapterDesc);
			if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
			if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Device)))) break;
		}

		//if you cannot create a device level 12.0, create a WARP device 
		if (adapter == nullptr)
		{
			m_Factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
			D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device));
		}

		//create an event handle to use for frame synchronization
		result = m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));
		m_FenceValue = 0;
		m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		//check 4X MSAA
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevels;
		msaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		msaaQualityLevels.SampleCount = 4;
		msaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		msaaQualityLevels.NumQualityLevels = 0;
		m_Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaQualityLevels, sizeof(msaaQualityLevels));
		m_Msaa4xQuality = msaaQualityLevels.NumQualityLevels;
		m_Msaa4xEnable = (m_Msaa4xQuality > 1) ? true : false;

		if (adapter != nullptr)
		{
			adapter->Release();
		}

		return true;
	}

	bool DirectX12::CreateCommandObjects()
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc;
		ZeroMemory(&queueDesc, sizeof(queueDesc));
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		HRESULT result = m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue));
		if (FAILED(result)) return false;

		result = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator));
		if (FAILED(result)) return false;

		result = m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator, nullptr, IID_PPV_ARGS(&m_CommandList));
		if (FAILED(result)) return false;

		m_CommandList->Close();

		return true;
	}

	void DirectX12::CreateSwapChain()
	{
		DXGI_SWAP_CHAIN_DESC1 scDesc;
		ZeroMemory(&scDesc, sizeof(scDesc));
		scDesc.Width = m_WindowInfo.m_Width;
		scDesc.Height = m_WindowInfo.m_Height;
		scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scDesc.SampleDesc.Count = m_Msaa4xEnable ? 4 : 1;
		scDesc.SampleDesc.Quality = m_Msaa4xEnable ? m_Msaa4xQuality - 1 : 0;
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.BufferCount = m_SwapChainBufferCount;
		scDesc.Scaling = DXGI_SCALING_NONE;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		scDesc.Flags = 0;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC scFullDesc;
		ZeroMemory(&scFullDesc, sizeof(scFullDesc));
		scFullDesc.RefreshRate.Numerator = 60;
		scFullDesc.RefreshRate.Denominator = 1;
		scFullDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		scFullDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		scFullDesc.Windowed = true;

		m_Factory->CreateSwapChainForHwnd(m_CommandQueue, m_WindowInfo.m_hwnd, &scDesc, &scFullDesc, nullptr,
			(IDXGISwapChain1**)(&m_SwapChain));

		m_CurrentSwapChainBuffer = m_SwapChain->GetCurrentBackBufferIndex();

		m_Factory->MakeWindowAssociation(m_WindowInfo.m_hwnd, DXGI_MWA_NO_ALT_ENTER);
	}

	bool DirectX12::CreateRtvDsvHeap()
	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvDHDesc;
		ZeroMemory(&rtvDHDesc, sizeof(rtvDHDesc));
		rtvDHDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvDHDesc.NumDescriptors = m_SwapChainBufferCount;
		rtvDHDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvDHDesc.NodeMask = 0;
		HRESULT result = m_Device->CreateDescriptorHeap(&rtvDHDesc, IID_PPV_ARGS(&m_RtvDescriptorHeap));
		if (FAILED(result)) return false;
		m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_DESCRIPTOR_HEAP_DESC dsvDHDesc;
		ZeroMemory(&dsvDHDesc, sizeof(dsvDHDesc));
		dsvDHDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvDHDesc.NumDescriptors = 1;
		dsvDHDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvDHDesc.NodeMask = 0;
		result = m_Device->CreateDescriptorHeap(&dsvDHDesc, IID_PPV_ARGS(&m_DsvDescriptorHeap));
		if (FAILED(result)) return false;
		m_DsvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		return true;
	}

	bool DirectX12::CreateRenderTargetViews()
	{
		HRESULT result;
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		for (UINT i = 0; i < m_SwapChainBufferCount; ++i)
		{
			result = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTargetBuffers[i]));
			if (FAILED(result)) return false;
			m_Device->CreateRenderTargetView(m_RenderTargetBuffers[i], nullptr, rtvHandle);
			rtvHandle.ptr += m_RtvDescriptorSize;
		}
		return true;
	}

	void DirectX12::CreateDepthStecilView()
	{
		D3D12_RESOURCE_DESC dsvDesc;
		ZeroMemory(&dsvDesc, sizeof(dsvDesc));
		dsvDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		dsvDesc.Alignment = 0;
		dsvDesc.Width = m_WindowInfo.m_Width;
		dsvDesc.Height = m_WindowInfo.m_Height;
		dsvDesc.DepthOrArraySize = 1;
		dsvDesc.MipLevels = 1;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.SampleDesc.Count = m_Msaa4xEnable ? 4 : 1;
		dsvDesc.SampleDesc.Quality = m_Msaa4xEnable ? m_Msaa4xQuality - 1 : 0;
		dsvDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		dsvDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_HEAP_PROPERTIES heapProperties;
		ZeroMemory(&heapProperties, sizeof(heapProperties));
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 1;
		heapProperties.VisibleNodeMask = 1;

		D3D12_CLEAR_VALUE clearValue;
		ZeroMemory(&clearValue, sizeof(clearValue));
		clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		m_Device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &dsvDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue, IID_PPV_ARGS(&m_DepthStencilBuffer));

		D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle = m_DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		m_Device->CreateDepthStencilView(m_DepthStencilBuffer, nullptr, cpuDescriptorHandle);
	}

	void DirectX12::SetViewAndScissor()
	{
		m_Viewport.TopLeftX = 0;
		m_Viewport.TopLeftY = 0;
		m_Viewport.Width = static_cast<float>(m_WindowInfo.m_Width);
		m_Viewport.Height = static_cast<float>(m_WindowInfo.m_Height);
		m_Viewport.MaxDepth = 1.0f;
		m_Viewport.MinDepth = 0.0f;

		m_ScissorRect = { 0, 0, static_cast<LONG>(m_WindowInfo.m_Width), static_cast<LONG>(m_WindowInfo.m_Height) };
	}

	void DirectX12::WaitForGpuCompelete()
	{
		m_FenceValue++;

		HRESULT result = m_CommandQueue->Signal(m_Fence, m_FenceValue);

		if (m_Fence->GetCompletedValue() < m_FenceValue)
		{
			result = m_Fence->SetEventOnCompletion(m_FenceValue, m_FenceEvent);
			WaitForSingleObject(m_FenceEvent, INFINITE);
		}
	}

	void DirectX12::BeginScene(const XMFLOAT4& color)
	{
		//Reset CommandAllocator, CommandList
		HRESULT result = m_CommandAllocator->Reset();
		result = m_CommandList->Reset(m_CommandAllocator, nullptr);

		//Indicate a state transition on the resource usage
		D3D12_RESOURCE_BARRIER resourceBarrier;
		resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		resourceBarrier.Transition.pResource = m_RenderTargetBuffers[m_CurrentSwapChainBuffer];
		resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		m_CommandList->ResourceBarrier(1, &resourceBarrier);

		//Set Viewport, scissor rect
		m_CommandList->RSSetViewports(1, &m_Viewport);
		m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

		//Clear render target view
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += (m_CurrentSwapChainBuffer * m_RtvDescriptorSize);
		float clearColor[4] = { color.x, color.y, color.z, color.w };
		m_CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		//Clear depth stencil view
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		m_CommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		//Connect RTV and DSV to OM
		m_CommandList->OMSetRenderTargets(1, &rtvHandle, true, &dsvHandle);
	}

	void DirectX12::EndScene()
	{
		//Indicate a state transition on the resource usage
		D3D12_RESOURCE_BARRIER resourceBarrier;
		resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		resourceBarrier.Transition.pResource = m_RenderTargetBuffers[m_CurrentSwapChainBuffer];
		resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		m_CommandList->ResourceBarrier(1, &resourceBarrier);

		//Done recording commands
		HRESULT result = m_CommandList->Close();

		//Add the command list to queue for excution
		ID3D12CommandList* commandLists[] = { m_CommandList };
		m_CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

		WaitForGpuCompelete();

		m_SwapChain->Present(0, 0);

		m_CurrentSwapChainBuffer = m_SwapChain->GetCurrentBackBufferIndex();
	}
}
