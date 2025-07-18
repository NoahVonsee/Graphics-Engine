#include "pch.h"

#define FAST_OBJ_IMPLEMENTATION (1)
#include "../../fast_obj/fast_obj.h"

#define EVALUATE(hr) if (!SUCCEEDED(hr)) { __debugbreak(); }

#include "MathFunctions.h"
   
static bool quit;

Engine::Engine(){
    EVALUATE(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_shaderCompiler)));
}

// Used to create a window and initialize DirectX 12
void Engine::Initialize(UINT width, UINT height, std::wstring windowName)
{
    quit = false;
    this->_width = width;
    this->_height = height;
    this->_windowName = windowName;

    _msg = {};

    InitWindow();
    InitDX();
}

// Used to render a frame 
void Engine::Render()
{
    HRESULT hr;
    if (!quit) {
        uint32_t bufferIndex = _frameCounter % _bufferCount;

        gpu_frame_data_t* currentFrameData = &_frameData[bufferIndex];

        WaitForSingleObject(currentFrameData->frameEndEvent, INFINITE);

        // Clear memory
        hr = currentFrameData->commandAllocator->Reset();
        EVALUATE(hr);

        _commandList->Reset(currentFrameData->commandAllocator, NULL);
        EVALUATE(hr);


        // barrier for begin of rendering
        D3D12_RESOURCE_BARRIER render_begin_barrier = {};
        render_begin_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        render_begin_barrier.Transition.pResource = _swapChainBuffer[bufferIndex];
        render_begin_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        render_begin_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

        // Set up viewport
        D3D12_VIEWPORT viewport = {};
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = _width;
        viewport.Height = _height;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        _commandList->RSSetViewports(1, &viewport);

        // Set up scissor rectangle
        D3D12_RECT scissorRect = {};
        scissorRect.bottom = _height;
        scissorRect.top = 0;
        scissorRect.left = 0;
        scissorRect.right = _width;

        _commandList->RSSetScissorRects(1, &scissorRect);

        _commandList->ResourceBarrier(1, &render_begin_barrier);

        // Set render target
        _commandList->OMSetRenderTargets(1, &_swapChainRTV[bufferIndex], TRUE, NULL);

        // Clear render target
        _commandList->ClearRenderTargetView(_swapChainRTV[bufferIndex], _background, 0, NULL);

        // Set root signature
        _commandList->SetGraphicsRootSignature(_rootSignature);

        for (int i = 0; i < _graphicsPipelineStates.size(); i++) {
            _commandList->SetPipelineState(_graphicsPipelineStates[i]);
            _commandList->SetDescriptorHeaps(1, &_bufferDescriptorHeap);
            _commandList->SetGraphicsRootDescriptorTable(0, _bufferDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

            _commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
            indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
            indexBufferView.SizeInBytes = _geometry[i].indexCount * sizeof(uint16_t);
            indexBufferView.Format = DXGI_FORMAT_R16_UINT;
            _commandList->IASetIndexBuffer(&indexBufferView);
            _commandList->DrawIndexedInstanced(_geometry[i].indexCount, 1, 0, 0, 0);
        }

        // barrier for end of rendering
        D3D12_RESOURCE_BARRIER render_end_barrier = {};
        render_end_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        render_end_barrier.Transition.pResource = _swapChainBuffer[bufferIndex];
        render_end_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        render_end_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

        _commandList->ResourceBarrier(1, &render_end_barrier);

        // Close command list
        hr = _commandList->Close();
        EVALUATE(hr);

        ID3D12CommandList* commandListsToExecute[] = { _commandList };
        _graphicsQueue->ExecuteCommandLists(_countof(commandListsToExecute), commandListsToExecute);

        _graphicsQueue->Signal(currentFrameData->frameEndFence, (UINT64)_frameCounter + 1);
        currentFrameData->frameEndFence->SetEventOnCompletion((UINT64)_frameCounter + 1, currentFrameData->frameEndEvent);

        _frameCounter++;

        // Present swap chain
        DXGI_PRESENT_PARAMETERS presentParams = { 0 };
        hr = _swapChain->Present1(1, 0, &presentParams);
        EVALUATE(hr);

        HandleMessages();
    }
    else {
        // Clear memory on application end
        ID3D12Fence* wfiFence;
        HANDLE wfiEvent;
        hr = _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&wfiFence));
        EVALUATE(hr);
        wfiEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        assert(wfiEvent);

        wfiFence->SetEventOnCompletion(1, wfiEvent);
        _graphicsQueue->Signal(wfiFence, 1);

        WaitForSingleObject(wfiEvent, INFINITE);

        wfiFence->Release();
        CloseHandle(wfiEvent);

        _commandList->Release();

        for (int i = 0; i < _countof(_frameData); i++) {
            _frameData[i].frameEndFence->Release();

            _frameData[i].commandAllocator->Reset();
            _frameData[i].commandAllocator->Release();

            CloseHandle(_frameData[i].frameEndEvent);
        }

        for (int i = 0; i < _graphicsPipelineStates.size(); i++) {
            _graphicsPipelineStates[i]->Release();
        }

        _rootSignature->Release();
        _graphicsQueue->Release();
        _RTVDescriptorHeap->Release();
        _graphicsQueue->Release();
        _device->Release();
        _D3DDebug->Release();
        _dxgiFactory->Release();
        _swapChain->Release();
        _shaderCompiler->Release();

        exit(0);
    }
}

// Sends commands to the window
void Engine::HandleMessages() {
    // Handle messages
    if (PeekMessage(&_msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&_msg);
        DispatchMessage(&_msg);

        if (_msg.message == WM_QUIT) {
            // Quit application
            exit(0);
        }
    }
}

// Used to create a window
void Engine::InitWindow()
{
    HINSTANCE hInstance = GetModuleHandle(NULL);

    // Register window class
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"WindowClass";
    RegisterClassEx(&wc);

    // Create a window
    _hwnd = CreateWindowEx(
        0,                              // Optional window styles
        L"WindowClass",                 // Window class
        _windowName.c_str(),            // Window title
        WS_OVERLAPPEDWINDOW,            // Window style

        CW_USEDEFAULT, CW_USEDEFAULT,   // Position
        this->_width, this->_height,    // Size

        NULL,                           // Parent window    
        NULL,                           // Menu
        hInstance,                      // Instance handle
        NULL                            // Additional application data
    );

    int nCmdShow = SW_SHOWNORMAL;

    // Show window if possible
    if (_hwnd != NULL) {
        ShowWindow(_hwnd, nCmdShow);

        std::cout << "Window opened:" << _hwnd << std::endl;
    }
    else {
        __debugbreak();
    }
}

void Engine::InitDX()
{
    HRESULT hr;

    //hr = DXGIGetDebugInterface(IID_PPV_ARGS(&_dxgiDebug));

    // Create D3D Debugger
    hr = D3D12GetDebugInterface(IID_PPV_ARGS(&_D3DDebug));
    _D3DDebug->EnableDebugLayer();

    // Get dxgiFacftory from adapter
    hr = CreateDXGIFactory(__uuidof(IDXGIFactory2), (void**)&_dxgiFactory);
    EVALUATE(hr);

    uint32_t bestAdapter;
    size_t bestDedicatedVMEM = 0;
    IDXGIAdapter* adapter[16];
    uint32_t adapterCount = 0;
    while (_dxgiFactory->EnumAdapters(adapterCount, &adapter[adapterCount]) != DXGI_ERROR_NOT_FOUND) {
        DXGI_ADAPTER_DESC desc;
        adapter[adapterCount]->GetDesc(&desc);

        wprintf(L"Adapter %u:\n", adapterCount);
        wprintf(L"%s:\n", desc.Description);
        wprintf(L"DedicatedVideoMemory: %llu bytes\n", desc.DedicatedVideoMemory);
        wprintf(L"DedicatedSystemMemory: %llu bytes\n", desc.DedicatedSystemMemory);
        wprintf(L"SharedSystemMemory: %llu bytes\n", desc.SharedSystemMemory);

        if (desc.DedicatedVideoMemory > bestDedicatedVMEM) {
            bestDedicatedVMEM = desc.DedicatedVideoMemory;
            bestAdapter = adapterCount;
        }
        adapterCount++;
    }

    printf("Using adapter: %u", bestAdapter);

    // Create D3D Device
    hr = D3D12CreateDevice(adapter[bestAdapter], D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&_device));
    EVALUATE(hr);

    // Create Command Queue
    D3D12_COMMAND_QUEUE_DESC graphicsQueueDesc = { D3D12_COMMAND_LIST_TYPE_DIRECT };
    hr = _device->CreateCommandQueue(&graphicsQueueDesc, IID_PPV_ARGS(&_graphicsQueue));
    EVALUATE(hr);
    
    // Set swap chain info
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

    // Set buffer amount
    swapChainDesc.BufferCount = _bufferCount;

    // Set width and height
    swapChainDesc.Width = _width;
    swapChainDesc.Height = _height;

    // Set pixel format to an 8-bit float4 
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    // Set the usage of the buffer as the render output
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    // Set swap mode to repeat sequentially
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

    // Disable anti-aliasing
    swapChainDesc.SampleDesc.Count = 1;

    // Create swap chain
    hr = _dxgiFactory->CreateSwapChainForHwnd(_graphicsQueue, _hwnd, &swapChainDesc, NULL, NULL, &_swapChain);
    EVALUATE(hr);

    // Create the descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC RTVDescriptorHeapDesc = { D3D12_DESCRIPTOR_HEAP_TYPE_RTV };
    RTVDescriptorHeapDesc.NumDescriptors = _bufferCount;
    hr = _device->CreateDescriptorHeap(&RTVDescriptorHeapDesc, IID_PPV_ARGS(&_RTVDescriptorHeap));
    EVALUATE(hr);

    D3D12_CPU_DESCRIPTOR_HANDLE nextDescriptor = _RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    size_t RTVDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // Create a render target view for every buffer in the swap chain 
    for (int i = 0; i < _countof(_swapChainRTV); i++) 
    {
        _swapChainRTV[i] = nextDescriptor;
        nextDescriptor.ptr += RTVDescriptorSize;

        hr = _swapChain->GetBuffer(i, IID_PPV_ARGS(&_swapChainBuffer[i]));
        EVALUATE(hr);

        D3D12_RENDER_TARGET_VIEW_DESC RTVDesc = { };
        RTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        RTVDesc.Texture2D.MipSlice = 0;
        RTVDesc.Texture2D.PlaneSlice = 0;

        _device->CreateRenderTargetView(_swapChainBuffer[i], &RTVDesc, _swapChainRTV[i]);
    }

    for (int i = 0; i < _countof(_frameData); i++) {
        hr = _device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_frameData[i].commandAllocator));
        EVALUATE(hr);

        hr = _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_frameData[i].frameEndFence));
        EVALUATE(hr);

        _frameData[i].frameEndEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
        assert(_frameData[i].frameEndEvent);
    }

    D3D12_DESCRIPTOR_RANGE cameraDataDescriptorRange = {};
    cameraDataDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    cameraDataDescriptorRange.NumDescriptors = 1;
    cameraDataDescriptorRange.BaseShaderRegister = 0;
    cameraDataDescriptorRange.RegisterSpace = 0;
    cameraDataDescriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_DESCRIPTOR_RANGE vertexDataDescriptorRange = {};
    vertexDataDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    vertexDataDescriptorRange.NumDescriptors = 1;
    vertexDataDescriptorRange.BaseShaderRegister = 0;
    vertexDataDescriptorRange.RegisterSpace = 0;
    vertexDataDescriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_DESCRIPTOR_RANGE descriptor_ranges[] = { vertexDataDescriptorRange, cameraDataDescriptorRange };

    D3D12_ROOT_PARAMETER vertexCameraDataParameter = { };
    vertexCameraDataParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    vertexCameraDataParameter.DescriptorTable.NumDescriptorRanges = _countof(descriptor_ranges);
    vertexCameraDataParameter.DescriptorTable.pDescriptorRanges = descriptor_ranges;
    vertexCameraDataParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = { 0 };
    rootSignatureDesc.NumParameters = 1;
    rootSignatureDesc.pParameters = &vertexCameraDataParameter;

    // Create root signature blob
    ID3DBlob* rootSignatureBlob;
    hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, NULL);
    EVALUATE(hr);

    // Create root signature
    hr = _device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&_rootSignature));
    EVALUATE(hr);

    for (int i = 0; i < _objects.size(); i++)
    {
        AddObject(_objects[i]);
    }

    DirectX::XMMATRIX worldMatrix = DirectX::XMMATRIX
        (
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );

    DirectX::XMFLOAT3 cameraPosition = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    DirectX::XMFLOAT3 targetPosition = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
    DirectX::XMFLOAT3 upDirection = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);

    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(
        DirectX::XMLoadFloat3(&cameraPosition),
        DirectX::XMLoadFloat3(&targetPosition),
        DirectX::XMLoadFloat3(&upDirection)
    );

    float aspectRatio = _width / _height;
    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(90, aspectRatio, 0.0001f, 100);

    DirectX::XMMATRIX wvpMatrix = worldMatrix * viewMatrix * projectionMatrix ;

    camera_data_t cameraData = *(camera_data_t*)&wvpMatrix;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    UINT16 geometryIndex = _geometry.size() - 1;
    D3D12_RESOURCE_DESC vertexBufferDesc = { };
    vertexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vertexBufferDesc.Alignment = 0;
    vertexBufferDesc.Width = sizeof(vertex_data_t) * _geometry[geometryIndex].vertexCount;
    vertexBufferDesc.Height = 1;
    vertexBufferDesc.DepthOrArraySize = 1;
    vertexBufferDesc.MipLevels = 1;
    vertexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    vertexBufferDesc.SampleDesc.Count = 1;
    vertexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    vertexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    hr = _device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
        &vertexBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        NULL,
        IID_PPV_ARGS(&vertexDataBuffer)
    );
    EVALUATE(hr);

    D3D12_RANGE vertexDataRange{ 0, vertexBufferDesc.Width };
    void* vertexDataMap;
    hr = vertexDataBuffer->Map(0, &vertexDataRange, &vertexDataMap);
    EVALUATE(hr);

    vertex_data_t* vertices = reinterpret_cast<vertex_data_t*>(vertexDataMap);
    size_t vertexSize = sizeof(vertex_data_t);

    for (size_t j = 0; j < _geometry[0].vertexCount; ++j) {
        vertices[j] = _geometry[0].vertices[j];
    }

    vertexDataBuffer->Unmap(0, &vertexDataRange);
    D3D12_RESOURCE_DESC indexBufferDesc = { };
    indexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    indexBufferDesc.Alignment = 0;
    indexBufferDesc.Width = sizeof(uint16_t) * _geometry[geometryIndex].indexCount;
    indexBufferDesc.Height = 1;
    indexBufferDesc.DepthOrArraySize = 1;
    indexBufferDesc.MipLevels = 1;
    indexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    indexBufferDesc.SampleDesc.Count = 1;
    indexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    indexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    hr = _device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
        &indexBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        NULL,
        IID_PPV_ARGS(&_indexBuffer)
    );
    EVALUATE(hr);

    D3D12_RANGE indexDataRange{ 0, indexBufferDesc.Width };
    void* indexDataMap;
    hr = _indexBuffer->Map(0, &indexDataRange, &indexDataMap);
    EVALUATE(hr);

    uint16_t* indices = reinterpret_cast<uint16_t*>(indexDataMap);
    size_t indexSize = sizeof(UINT);

    for (size_t j = 0; j < _geometry[0].indexCount; ++j) {
        indices[j] = _geometry[0].indices[j];
    }

    _indexBuffer->Unmap(0, &indexDataRange);

    D3D12_RESOURCE_DESC cameraBufferDesc = { };
    cameraBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    cameraBufferDesc.Alignment = 0;
    cameraBufferDesc.Width = 256;
    cameraBufferDesc.Height = 1;
    cameraBufferDesc.DepthOrArraySize = 1;
    cameraBufferDesc.MipLevels = 1;
    cameraBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    cameraBufferDesc.SampleDesc.Count = 1;
    cameraBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    cameraBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    hr = _device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
        &cameraBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        NULL,
        IID_PPV_ARGS(&cameraDataBuffer)
    );
    EVALUATE(hr);

    D3D12_RANGE cameraDataRange{ 0, sizeof(cameraData) };
    void* cameraMap;
    hr = cameraDataBuffer->Map(0, &cameraDataRange, &cameraMap);
    EVALUATE(hr);
    memcpy(cameraMap, &cameraData, sizeof(cameraData));
    cameraDataBuffer->Unmap(0, &cameraDataRange);

    // Create the descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC bufferDescriptorHeapDesc = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };
    bufferDescriptorHeapDesc.NumDescriptors = _bufferCount - 1;
    bufferDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    hr = _device->CreateDescriptorHeap(&bufferDescriptorHeapDesc, IID_PPV_ARGS(&_bufferDescriptorHeap));
    EVALUATE(hr);

    D3D12_CPU_DESCRIPTOR_HANDLE bufferNextDescriptorHandle = _bufferDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_SHADER_RESOURCE_VIEW_DESC vertexDataSRVDesc = {};
    vertexDataSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
    vertexDataSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    vertexDataSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    vertexDataSRVDesc.Buffer.FirstElement = 0;
    vertexDataSRVDesc.Buffer.NumElements = _geometry[0].vertexCount;
    vertexDataSRVDesc.Buffer.StructureByteStride = sizeof(vertex_data_t);
    _device->CreateShaderResourceView(vertexDataBuffer, &vertexDataSRVDesc, bufferNextDescriptorHandle);
    bufferNextDescriptorHandle.ptr += _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_CONSTANT_BUFFER_VIEW_DESC cameraDataCBVDesc = {};
    cameraDataCBVDesc.BufferLocation = cameraDataBuffer->GetGPUVirtualAddress();
    cameraDataCBVDesc.SizeInBytes = 256;
    _device->CreateConstantBufferView(&cameraDataCBVDesc, bufferNextDescriptorHandle);
    bufferNextDescriptorHandle.ptr += _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Create graphics command list
    hr = _device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _frameData[0].commandAllocator, NULL, IID_PPV_ARGS(&_commandList));
    EVALUATE(hr);

    _commandList->Close();
}

void Engine::LoadObject(Object obj)
{
    _objects.push_back(obj);
}

void Engine::LoadObjects(std::vector<Object> objs)
{
    for (int i = 0; i < objs.size(); i++)
    {
        _objects.push_back(objs[i]);
    }
}

void Engine::AddObject(Object obj)
{
    HRESULT hr;

    _geometry.push_back(LoadGeometry(obj.GetModelFile()));

    //D3D12_SHADER_BYTECODE vsBytecode = LoadShader(_shaderCompiler, obj.GetVertShaderFile(), k_shader_stage_vertex);
    //D3D12_SHADER_BYTECODE psBytecode = LoadShader(_shaderCompiler, obj.GetPixelShaderFile(), k_shader_stage_pixel);

    D3D12_SHADER_BYTECODE vsMeshBytecode = LoadShader(_shaderCompiler, obj.GetMeshVSFile(), k_shader_stage_vertex);
    D3D12_SHADER_BYTECODE psMeshBytecode = LoadShader(_shaderCompiler, obj.GetMeshPSFile(), k_shader_stage_pixel);

    // Create graphics pipeline state 
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {};
    graphicsPipelineStateDesc.pRootSignature = _rootSignature;
    graphicsPipelineStateDesc.VS = vsMeshBytecode;
    graphicsPipelineStateDesc.PS = psMeshBytecode;
    graphicsPipelineStateDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    graphicsPipelineStateDesc.BlendState.RenderTarget[0].BlendEnable = false;
    graphicsPipelineStateDesc.SampleMask = 1;
    graphicsPipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    graphicsPipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    graphicsPipelineStateDesc.RasterizerState.FrontCounterClockwise = false;
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    graphicsPipelineStateDesc.SampleDesc.Count = 1;

    ID3D12PipelineState* graphicsPipelineState;
    hr = _device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
    EVALUATE(hr);

    _graphicsPipelineStates.push_back(graphicsPipelineState);
}

D3D12_SHADER_BYTECODE Engine::LoadShader(IDxcCompiler3* compiler, const char* filename, _shader_stage_t stage) {
    FILE* file = NULL;
    fopen_s(&file, filename, "rb");
    assert(file);

    fseek(file, 0, SEEK_END);
    size_t sourceSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    void* sourceBuffer = calloc(1, sourceSize);
    assert(sourceBuffer);

    fread_s(sourceBuffer, sourceSize, 1, sourceSize, file);
    fclose(file);

    LPCWSTR shader_target = stage == k_shader_stage_vertex ? L"vs_6_5" : L"ps_6_5";

    LPCWSTR pszArgs[] =
    {
        L"-E", L"main",              // Entry point.
        L"-T", shader_target,        // Target.
        L"-Zs",                      // Enable debug information (slim format)
        L"-Qstrip_reflect",          // Strip reflection into a separate blob. 
    };

    DxcBuffer sourceDxcBuffer;
    sourceDxcBuffer.Ptr = sourceBuffer;
    sourceDxcBuffer.Size = sourceSize;
    sourceDxcBuffer.Encoding = DXC_CP_ACP;

    HRESULT hr;
    IDxcResult* compileResult;
    hr = compiler->Compile(&sourceDxcBuffer, pszArgs, _countof(pszArgs), NULL , IID_PPV_ARGS(&compileResult));
    EVALUATE(hr);

    IDxcBlobUtf16* shaderName;
    HRESULT compileStatus;
    compileResult->GetStatus(&compileStatus);

    if (!SUCCEEDED(compileStatus)) {
        IDxcBlobUtf8* errorBlob;
        compileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errorBlob), &shaderName);

        printf("\nDXC compilation failed:\n%s", errorBlob->GetStringPointer());

        __debugbreak();
        exit(0);
    }
    
    EVALUATE(compileStatus);

    IDxcBlob* bytecodeBlob;
    compileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&bytecodeBlob), &shaderName);

    D3D12_SHADER_BYTECODE bytecode;
    bytecode.BytecodeLength = bytecodeBlob->GetBufferSize();
    bytecode.pShaderBytecode = bytecodeBlob->GetBufferPointer();

    compileResult->Release();
    //bytecodeBlob->Release();

    return bytecode;
}

geometry_t Engine::LoadGeometry(const char* filename)
{
    geometry_t geometry{};

    fastObjMesh* mesh = fast_obj_read(filename);
    assert(mesh);

    uint32_t totalOutputIndices = 0;
    for (uint32_t i = 0; i < mesh->face_count; i++) {
        uint32_t faceIndexCount = mesh->face_vertices[i];
        totalOutputIndices += (faceIndexCount - 2) * 3;
    }

    uint16_t* outputIndices = (uint16_t*)calloc(totalOutputIndices, sizeof(uint16_t));
    uint32_t outputIndex = 0;
    assert(totalOutputIndices <= UINT16_MAX);

    uint16_t outputVertex = 0;
    vertex_data_t* output = (vertex_data_t*)calloc(totalOutputIndices, sizeof(vertex_data_t));

   uint32_t baseIndex = 0;
    for (uint32_t i = 0; i < mesh->face_count; i++) {
        uint32_t faceIndexCount = mesh->face_vertices[i];

        for (uint32_t j = 1; j < faceIndexCount - 1; j++) {
            fastObjIndex index0 = mesh->indices[baseIndex + 0];
            fastObjIndex index1 = mesh->indices[baseIndex + j];
            fastObjIndex index2 = mesh->indices[baseIndex + j + 1];

            output[outputVertex].position = {
                mesh->positions[3 * index0.p + 0],
                mesh->positions[3 * index0.p + 1],
                mesh->positions[3 * index0.p + 2],
            };
            output[outputVertex].normal = {
               mesh->normals[3 * index0.n + 0],
               mesh->normals[3 * index0.n + 1],
               mesh->normals[3 * index0.n + 2],
            };
            outputIndices[outputIndex++] = outputVertex++;
            std::cout << outputIndices[outputIndex];

            output[outputVertex].position = {
                mesh->positions[3 * index1.p + 0],
                mesh->positions[3 * index1.p + 1],
                mesh->positions[3 * index1.p + 2],
            };
            output[outputVertex].normal = {
               mesh->normals[3 * index1.n + 0],
               mesh->normals[3 * index1.n + 1],
               mesh->normals[3 * index1.n + 2],
            };
            outputIndices[outputIndex++] = outputVertex++;
            std::cout << outputIndices[outputIndex];

            output[outputVertex].position = {
                mesh->positions[3 * index2.p + 0],
                mesh->positions[3 * index2.p + 1],
                mesh->positions[3 * index2.p + 2],
            };
            output[outputVertex].normal = {
               mesh->normals[3 * index2.n + 0],
               mesh->normals[3 * index2.n + 1],
               mesh->normals[3 * index2.n + 2],
            };
            outputIndices[outputIndex++] = outputVertex++;
            std::cout << outputIndices[outputIndex];
        }
        baseIndex += faceIndexCount;
    }

    assert(outputIndex == totalOutputIndices);
    assert(outputVertex == totalOutputIndices);

    geometry.indexCount = outputIndex;
    geometry.indices = outputIndices;
    geometry.vertexCount = outputVertex;
    geometry.vertices = output;

    return geometry;
}

void Engine::SetBackground(float background[4]) {
    memcpy(this->_background, background, sizeof(float)*4);
}

LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        quit = true;
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}