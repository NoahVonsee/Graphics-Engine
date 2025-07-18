#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <d3d12.h>
#include <dxgi1_2.h>
#include <dxcapi.h>
#include <dxgidebug.h>

#include <vector>
#include <string>

#include "Object.h"
#include "Types.h"

#include <DirectXMath.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxcompiler.lib")

class Engine
{

public:
	Engine();
	void Initialize(UINT width, UINT height, std::wstring name);
	void Render(); 
	void SetBackground(float background[4]);
	void LoadObject(Object obj);
	void LoadObjects(std::vector<Object> objs);

private:
	UINT _width;
	UINT _height; 
	std::wstring _windowName;
	float _background[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	void InitWindow();
	void InitDX();
	void HandleMessages();
	void AddObject(Object obj);
	D3D12_SHADER_BYTECODE LoadShader(IDxcCompiler3* compiler, const char* filename, _shader_stage_t stage);
	geometry_t LoadGeometry(const char* filename);

	static const int _bufferCount = 3;
	IDxcCompiler3* dxCompiler;
	MSG _msg;
	HWND _hwnd;
	ID3D12Device* _device;
	ID3D12CommandQueue* _graphicsQueue;
	IDXGISwapChain1* _swapChain;
	ID3D12RootSignature* _rootSignature;
    D3D12_CPU_DESCRIPTOR_HANDLE _swapChainRTV[_bufferCount];
	std::vector<ID3D12PipelineState*> _graphicsPipelineStates;
	ID3D12GraphicsCommandList* _commandList;
	ID3D12Resource* _swapChainBuffer[_bufferCount];
	IDxcCompiler3* _shaderCompiler;
	ID3D12DescriptorHeap* _RTVDescriptorHeap;
	ID3D12Debug* _D3DDebug;
	IDXGIFactory2* _dxgiFactory;
	gpu_frame_data_t _frameData[_bufferCount];
	uint32_t _frameCounter = 0;
	ID3D12Resource* _indexBuffer;
	ID3D12Resource* vertexDataBuffer;
	ID3D12Resource* cameraDataBuffer;
	ID3D12DescriptorHeap* _bufferDescriptorHeap;
	std::vector<geometry_t> _geometry;
	std::vector<Object> _objects;
};

LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);