#include "CommonHeaders.h"

void Update();

Engine graphicsEngine;
bool windowClosed;

UINT width = 1920;
UINT height = 1080;

Object monkey = Object(
    "Shaders/TriangleVertShader.hlsl",
    "Shaders/TrianglePixelShader.hlsl",
    "Models/Monkey.obj"
);

Object cube = Object(
    "Shaders/TriangleVertShader.hlsl",
    "Shaders/TrianglePixelShader.hlsl",
    "Models/Cube.obj"
);

Object vertex = Object(
    "Shaders/TriangleVertShader.hlsl",
    "Shaders/TrianglePixelShader.hlsl",
    "Models/vertex.obj"
);

Object all = Object(
    "Shaders/TriangleVertShader.hlsl",
    "Shaders/TrianglePixelShader.hlsl",
    "Models/all.obj"
);

int main() 
{
    graphicsEngine.LoadObject(all);
    graphicsEngine.Initialize(width, height, L"DirectX 12 Render Engine");

    float background[] = { 0.0f, 0.5f, 0.5f, 1.0f };
    graphicsEngine.SetBackground(background);

    while (true) {
        Update();

        graphicsEngine.Render();
    }
}

void Update() {
	//std::cout << ".";
}