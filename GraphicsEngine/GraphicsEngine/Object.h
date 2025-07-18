#pragma once

class Object
{
public:
	Object(const char* vertShaderFile, const char* pixelShaderFile, const char* modelFile);

	~Object();

	const char* GetVertShaderFile();
	const char* GetPixelShaderFile();
	const char* GetModelFile();

	const char* GetMeshVSFile();
	const char* GetMeshPSFile();

private:
	const char* vertShaderFile;
	const char* pixelShaderFile;
	const char* modelFile;
	const char* vsMeshFile = "../GraphicsEngine/VertShader.hlsl";
	const char* psMeshFile = "../GraphicsEngine/PixelShader.hlsl";
};

