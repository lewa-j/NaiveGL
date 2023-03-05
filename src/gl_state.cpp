#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"

void gl_state::init(int window_w, int window_h)
{
	error_bits = 0;

	begin_primitive_mode = -1;
	begin_vertex_count = 0;
	vertex = { glm::vec4(0, 0, 0, 1), glm::vec4(0, 0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec4(0, 0, 0, 1), true };
	edge_flag = true;
	current_tex_coord = glm::vec4(0, 0, 0, 1);
	current_normal = glm::vec3(0, 0, 1);
	current_color = glm::vec4(1, 1, 1, 1);

	viewport.width = window_w;
	viewport.height = window_h;
	viewport.center_x = window_w / 2;
	viewport.center_y = window_h / 2;
	viewport.dnear = 0;
	viewport.dfar = 1;

	matrix_mode = GL_MODELVIEW;
	modelview_sp = 0;
	projection_sp = 0;
	texture_mtx_sp = 0;
	modelview_stack[0] = glm::mat4(1);
	projection_stack[0] = glm::mat4(1);
	texture_mtx_stack[0] = glm::mat4(1);

	normalize = false;

	for (int i = 0; i < 4; i++)
	{
		texgen[i].enabled = false;
		texgen[i].mode = GL_EYE_LINEAR;
		texgen[i].eye_plane = glm::vec4(0, 0, 0, 0);
		texgen[i].object_plane = glm::vec4(0, 0, 0, 0);
	}
	texgen[0].eye_plane = glm::vec4(1, 0, 0, 0);
	texgen[1].eye_plane = glm::vec4(0, 1, 0, 0);
	texgen[0].object_plane = glm::vec4(1, 0, 0, 0);
	texgen[1].object_plane = glm::vec4(0, 1, 0, 0);

	enabled_clipplanes = 0;
	for (int i = 0; i < gl_max_user_clip_planes; i++)
		clipplanes[i] = glm::vec4(0);

	raster_pos.coords = glm::vec4{ 0,0,0,1 };
	raster_pos.distance = 0;
	raster_pos.valid = true;
	raster_pos.color = glm::vec4{ 0,0,0,1 };
	raster_pos.tex_coord = glm::vec4{ 1,1,1,1 };

	lighting_enabled = false;
	front_face_ccw = true;
	for (int i = 0; i < 2; i++)
	{
		materials[i].ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1);
		materials[i].diffuse = glm::vec4(0.8f, 0.8f, 0.8f, 1);
		materials[i].specular = glm::vec4(0, 0, 0, 1);
		materials[i].emission = glm::vec4(0, 0, 0, 1);
		materials[i].shininess = 0.0f;
	}
	enabled_lights = 0;
	for (int i = 0; i < gl_max_lights; i++)
	{
		lights[i].ambient = glm::vec4(0, 0, 0, 1);
		lights[i].diffuse = glm::vec4(0, 0, 0, 1);
		lights[i].specular = glm::vec4(0, 0, 0, 1);
		lights[i].position = glm::vec4(0, 0, 1, 0);
		lights[i].spot_direction = glm::vec3(0, 0, -1);
		lights[i].spot_exponent = 0.0f;
		lights[i].spot_cutoff = 180.0f;
		lights[i].attenuation[0] = 1.0f;//const
		lights[i].attenuation[1] = 0.0f;//linear
		lights[i].attenuation[2] = 0.0f;//quad
	}
	lights[0].diffuse = glm::vec4(1, 1, 1, 1);
	lights[0].specular = glm::vec4(1, 1, 1, 1);

	light_model_ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1);
	light_model_local_viewer = false;
	light_model_two_side = false;

	color_material = false;
	color_material_face = GL_FRONT_AND_BACK;
	color_material_mode = GL_AMBIENT_AND_DIFFUSE;

	shade_model_flat = false;
}

void gl_state::destroy()
{
}

void APIENTRY glEnable(GLenum cap)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (cap == GL_NORMALIZE)
	{
		gs->normalize = true;
	}
	else if (cap >= GL_TEXTURE_GEN_S && cap <= GL_TEXTURE_GEN_Q)
	{
		gs->texgen[cap - GL_TEXTURE_GEN_S].enabled = true;
	}
	else if (cap >= GL_CLIP_PLANE0 && cap < (GL_CLIP_PLANE0 + gl_max_user_clip_planes))
	{
		gs->enabled_clipplanes |= (1 << (cap - GL_CLIP_PLANE0));
	}
	else if (cap == GL_LIGHTING)
	{
		gs->lighting_enabled = true;
	}
	else if (cap >= GL_LIGHT0 && cap < (GL_LIGHT0 + gl_max_lights))
	{
		gs->enabled_lights |= (1 << (cap - GL_LIGHT0));
	}
	else if (cap == GL_COLOR_MATERIAL)
	{
		bool old = gs->color_material;
		gs->color_material = true;
		if (!old)
			gs->update_color_material();
	}
	else
	{
		gl_set_error_a(GL_INVALID_ENUM, cap);
		return;
	}
}

void APIENTRY glDisable(GLenum cap)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (cap == GL_NORMALIZE)
	{
		gs->normalize = false;
	}
	else if (cap >= GL_TEXTURE_GEN_S && cap <= GL_TEXTURE_GEN_Q)
	{
		gs->texgen[cap - GL_TEXTURE_GEN_S].enabled = false;
	}
	else if (cap >= GL_CLIP_PLANE0 && cap < (GL_CLIP_PLANE0 + gl_max_user_clip_planes))
	{
		gs->enabled_clipplanes &= ~(1 << (cap - GL_CLIP_PLANE0));
	}
	else if (cap == GL_LIGHTING)
	{
		gs->lighting_enabled = false;
	}
	else if (cap >= GL_LIGHT0 && cap < (GL_LIGHT0 + gl_max_lights))
	{
		gs->enabled_lights &= ~(1 << (cap - GL_LIGHT0));
	}
	else if (cap == GL_COLOR_MATERIAL)
	{
		gs->color_material = false;
	}
	else
	{
		gl_set_error_a(GL_INVALID_ENUM, cap);
		return;
	}
}

const char *APIENTRY glGetString(GLenum name)
{
	switch (name)
	{
	case GL_VENDOR:
		return "lewa_j";
	case GL_RENDERER:
		return "NaiveGL";
	case GL_VERSION:
		return "1.0.0";
	case GL_EXTENSIONS:
		return "";
	default:
		gl_set_error_a(GL_INVALID_ENUM, name);
		return "";
	}
}

static const char *error_to_str(GLenum e)
{
	switch (e)
	{
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_STACK_OVERFLOW:
		return "GL_STACK_OVERFLOW";
	case GL_STACK_UNDERFLOW:
		return "GL_STACK_UNDERFLOW";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	}
	return "?";
}

#if DEBUG_LOG
#define gl_log printf
#else
#define gl_log()
#endif

void gl_set_error_a_(GLenum error, GLenum arg, const char *func)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	if (error < GL_INVALID_ENUM || error > GL_OUT_OF_MEMORY)
		return;

	gl_log("%s: error %s 0x%X\n", func, error_to_str(error), arg);

	gs->error_bits |= (1 << (error - GL_INVALID_ENUM));
}

void gl_set_error_(GLenum error, const char *func)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	if (error < GL_INVALID_ENUM || error > GL_OUT_OF_MEMORY)
		return;

	gl_log("%s: error %s\n", func, error_to_str(error));

	gs->error_bits |= (1 << (error - GL_INVALID_ENUM));
}

GLenum APIENTRY glGetError()
{
	gl_state *gs = gl_current_state();
	if (!gs) return 0;

	if (gs->error_bits == 0)
		return GL_NO_ERROR;

	for (int i = 0; i < 6; i++)
	{
		if (gs->error_bits & (1 << i))
		{
			gs->error_bits &= ~(1 << i);
			return GL_INVALID_ENUM + i;
		}
	}

	return GL_NO_ERROR;
}