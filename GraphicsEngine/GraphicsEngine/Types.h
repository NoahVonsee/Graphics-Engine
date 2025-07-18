#include <cstdint>
#include <d3d12.h>

constexpr float pi = 3.1415926535897932384626433832795f;
constexpr float Epsilon = 0.0001f; 

typedef struct _vec2f_t {
	float x;
	float y;
} vec2f_t;

typedef struct _vec3f_t {
	float x;
	float y;
	float z;
} vec3f_t;

typedef struct _vec4f_t {
	float x;
	float y;
	float z;
	float w;
} vec4f_t;

typedef struct _mat4f_t {
	float m[4][4];
} mat4f_t;

typedef struct _vertex_data_t {
	vec3f_t position;
	vec3f_t normal;
	vec2f_t texcoord;
} vertex_data_t;

typedef struct _geometry_t {
	uint32_t indexCount;
	uint16_t* indices;

	uint32_t vertexCount;
	_vertex_data_t* vertices;
} geometry_t;

typedef struct _camera_data_t {
	//_mat4f_t WVPMatrix;

	vec4f_t clipFromViewX;
	vec4f_t clipFromViewY;
	vec4f_t clipFromViewZ;
	vec4f_t clipFromViewW;
} camera_data_t;

typedef struct _gpu_frame_data_t {
	ID3D12CommandAllocator* commandAllocator;
	ID3D12Fence* frameEndFence;
	HANDLE frameEndEvent;
	uint32_t frameBufferIndex;
} gpu_frame_data_t;

typedef enum _shader_stage_t {
	k_shader_stage_vertex,
	k_shader_stage_pixel,
} shader_stage_t;

