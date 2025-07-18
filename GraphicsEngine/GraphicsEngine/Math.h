//#include <cmath>
//#include <math.h>
//
//constexpr float pi = 3.1415926535897932384626433832795f;
//constexpr float Epsilon = 0.0001f;
//
//typedef struct _vec2f_t {
//	float x;
//	float y;
//} vec2f_t;
//
//typedef struct _vec3f_t {
//	float x;
//	float y;
//	float z;
//} vec3f_t;
//
//typedef struct _vec4f_t {
//	float x;
//	float y;
//	float z;
//	float w;
//} vec4f_t;
//
//typedef struct _mat4f_t {
//	float m[4][4];
//} mat4f_t;
//
//typedef struct _vertex_data_t {
//	vec3f_t position;
//	vec3f_t normal;
//	vec3f_t texcoord;
//} vertex_data_t;
//
//typedef struct _geometry_t {
//	uint32_t indexCount;
//	uint16_t* indices;
//
//	uint32_t vertexCount;
//	_vertex_data_t* vertices;
//} geometry_t;
//
//typedef struct _camera_data_t {
//	vec4f_t clipFromViewX;
//	vec4f_t clipFromViewY;
//	vec4f_t clipFromViewZ;
//	vec4f_t clipFromViewW;
//} camera_data_t;
//
//typedef struct _gpu_frame_data_t {
//	ID3D12CommandAllocator* commandAllocator;
//	ID3D12Fence* frameEndFence;
//	HANDLE frameEndEvent;
//	uint32_t frameBufferIndex;
//} gpu_frame_data_t;
//
//typedef enum _shader_stage_t {
//	k_shader_stage_vertex,
//	k_shader_stage_pixel,
//} shader_stage_t;
//
//
//inline bool IsTheSameAs(float value, float other)
//{
//	return std::abs(value - other) < Epsilon;
//}
//
//vec4f_t VEC4_CONSTRUCT(int x, int y, int z, int w) {
//	vec4f_t v4;
//
//	v4.x;
//	v4.y;
//	v4.z;
//	v4.w;
//
//	return v4;
//}
//
//vec3f_t vec4f_xyz(vec4f_t v4) {
//	vec3f_t v3;
//
//	v3.x = v4.x;
//	v3.y = v4.y;
//	v3.z = v4.z;
//
//	return v3;
//}
//
//float vec4f_dot(const vec4f_t a, const vec4f_t b) {
//	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
//}
//
//vec4f_t vec4f_from_vec3f(vec3f_t v3, float w) {
//	vec4f_t v4;
//
//	v4.x = v3.x;
//	v4.y = v3.y;
//	v4.z = v3.z;
//	v4.w = w;
//
//	return v4;
//}
//
//inline void mat4f_set_identity(mat4f_t* out_m) {
//	for (int row = 0; row < 4; row++) {
//		for (int col = 0; col < 4; col++) {
//			out_m->m[row][col] = (row == col) ? 1.0f : 0.0f;
//		}
//	}
//}
//
//inline void mat4f_transpose(mat4f_t* inout_m) {
//	for (int row = 0; row < 4; row++) {
//		for (int col = 0; col < 4; col++) {
//			float temp = inout_m->m[col][row];
//			inout_m->m[row][col] = inout_m->m[col][row];
//			inout_m->m[col][row] = temp;
//		}
//	}
//}
//
//inline void mat4f_make_translation(vec3f_t translation, mat4f_t* out_m) {
//	mat4f_set_identity(out_m);
//
//	out_m->m[0][3] = translation.x;
//	out_m->m[1][3] = translation.y;
//	out_m->m[2][3] = translation.z;
//}
//
//inline vec4f_t mat4f_get_row(const mat4f_t* m, int row) {
//	return VEC4_CONSTRUCT(
//		m->m[row][0],
//		m->m[row][1],
//		m->m[row][2],
//		m->m[row][3]
//	);
//}
//
//inline vec4f_t mat4f_get_col(const mat4f_t* m, int col) {
//	return VEC4_CONSTRUCT(
//		m->m[0][col],
//		m->m[1][col],
//		m->m[2][col],
//		m->m[3][col]
//	);
//}
//
//inline vec4f_t mat4f_transform_vec4f(const mat4f_t* m, vec4f_t v) {
//	vec4f_t result;
//	result.x = vec4f_dot(mat4f_get_row(m, 0), v);
//	result.y = vec4f_dot(mat4f_get_row(m, 1), v);
//	result.z = vec4f_dot(mat4f_get_row(m, 2), v);
//	result.w = vec4f_dot(mat4f_get_row(m, 3), v);
//	return result;
//}
//
//inline vec3f_t mat4f_transform_point_vec3f(const mat4f_t* m, vec3f_t v) {
//	vec4f_t v4 = vec4f_from_vec3f(v, 1.0f);
//	return vec4f_xyz(mat4f_transform_vec4f(m, v4));
//}
//
//inline vec3f_t mat4f_transform_vector_vec3f(const mat4f_t* m, vec3f_t v) {
//	vec4f_t v4 = vec4f_from_vec3f(v, 0.0f);
//	return vec4f_xyz(mat4f_transform_vec4f(m, v4));
//}
//
//inline void mat4f_multiply(const mat4f_t* a, const mat4f_t* b, mat4f_t* out_m) {
//	out_m->m[0][0] = vec4f_dot(mat4f_get_row(a, 0), mat4f_get_col(b, 0));
//	out_m->m[0][1] = vec4f_dot(mat4f_get_row(a, 0), mat4f_get_col(b, 1));
//	out_m->m[0][2] = vec4f_dot(mat4f_get_row(a, 0), mat4f_get_col(b, 2));
//	out_m->m[0][3] = vec4f_dot(mat4f_get_row(a, 0), mat4f_get_col(b, 3));
//
//	out_m->m[1][0] = vec4f_dot(mat4f_get_row(a, 1), mat4f_get_col(b, 0));
//	out_m->m[1][1] = vec4f_dot(mat4f_get_row(a, 1), mat4f_get_col(b, 1));
//	out_m->m[1][2] = vec4f_dot(mat4f_get_row(a, 1), mat4f_get_col(b, 2));
//	out_m->m[1][3] = vec4f_dot(mat4f_get_row(a, 1), mat4f_get_col(b, 3));
//}
//
//inline bool mat4f_almost_equals(const mat4f_t* a, const mat4f_t* b) {
//	for (int row = 0; row < 4; row++) {
//		for (int col = 0; col < 4; col++) {
//			if (!IsTheSameAs(a->m[row][col], b->m[row][col])) {
//				return false;
//			}
//		}
//	}
//	return true;
//}
//
//double degreesToRadians(double degrees) {
//	return degrees * pi / 180.0;
//}
//
//const float clamp(float value, float min, float max) {
//
//	if (value < min)
//		return min;
//	if (value > max)
//		return max;
//
//	return value;
//}
//
//inline void mat4f_make_perspective_and_inverse(mat4f_t* out_m, mat4f_t* out_inv_m, float vfov_radians, float aspect, float near_plane) {
//	const float k_min_fov_radians = degreesToRadians(1.0f);
//	const float k_max_fov_radians = degreesToRadians(179.0f);
//	vfov_radians = clamp(vfov_radians, k_min_fov_radians, k_max_fov_radians);
//	near_plane = __max(near_plane, 0.001f);
//	aspect = __max(aspect, 0.001f);
//
//	float cot_half_fov = 1.0f / tanf(0.5f * vfov_radians);
//
//	memset(out_m, 0, sizeof(*out_m));
//
//	out_m->m[0][0] = cot_half_fov / aspect;
//	out_m->m[1][1] = cot_half_fov;
//	out_m->m[3][2] = near_plane;
//	out_m->m[2][3] = -1.0f;
//
//	memset(out_inv_m, 0, sizeof(*out_inv_m));
//
//	out_inv_m->m[0][0] = aspect / cot_half_fov;
//	out_inv_m->m[1][1] = 1.0f / cot_half_fov;
//	out_inv_m->m[3][2] = -1.0f;
//	out_inv_m->m[2][3] = 1.0f / near_plane;
//}