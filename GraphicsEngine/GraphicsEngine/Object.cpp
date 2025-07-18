#include "pch.h"


Object::Object(const char* vertShaderFile, const char* pixelShaderFile, const char* modelFile)
{
	this->vertShaderFile = vertShaderFile;
	this->pixelShaderFile = pixelShaderFile;
	this->modelFile = modelFile;
}

Object::~Object()
{
	
}

const char* Object::GetVertShaderFile()
{
	return vertShaderFile;
}

const char* Object::GetPixelShaderFile()
{
	return pixelShaderFile;
}

const char* Object::GetModelFile()
{
	return modelFile;
}

const char* Object::GetMeshVSFile()
{
	return vsMeshFile;
}

const char* Object::GetMeshPSFile()
{
	return psMeshFile;
}
