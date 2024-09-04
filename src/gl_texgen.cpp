#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"

glm::vec4 gl_state::get_vertex_texcoord(glm::vec4 tr, const glm::vec3 &norm, const glm::vec4 &vertex_object, const glm::vec4 &vertex_eye)
{
	for (int i = 0; i < 4; i++)
	{
		const gl_state::texGen &tg = texgen[i];
		if (!tg.enabled || tg.mode == GL_SPHERE_MAP)
			continue;

		if (tg.mode == GL_EYE_LINEAR)
		{
			tr[i] = glm::dot(tg.eye_plane, vertex_eye);
		}
		else if (tg.mode == GL_OBJECT_LINEAR)
		{
			tr[i] = glm::dot(tg.object_plane, vertex_object);
		}
	}

	if ((texgen[0].enabled && texgen[0].mode == GL_SPHERE_MAP)
		|| (texgen[1].enabled && texgen[1].mode == GL_SPHERE_MAP))
	{
		glm::vec3 u = glm::normalize(glm::vec3(vertex_eye));
		glm::vec3 eye_normal = get_eye_normal(norm);
		glm::vec3 r = u - 2.0f * eye_normal * glm::dot(eye_normal, u);//same as reflect(u, eye_normal)
		r.z += 1;
		float im = 1.0f / (2.0f * glm::length(r));//for normalization

		if (texgen[0].enabled && texgen[0].mode == GL_SPHERE_MAP)
			tr.x = r.x * im + 0.5f;
		if (texgen[1].enabled && texgen[1].mode == GL_SPHERE_MAP)
			tr.y = r.y * im + 0.5f;
	}

	return get_mtx_texture() * tr;
}

#define VALIDATE_TEXGEN_COORD \
if (coord < GL_S || coord > GL_Q)\
{\
	gl_set_error_a(GL_INVALID_ENUM, coord);\
	return;\
}

#define VALIDATE_TEXGEN_V \
VALIDATE_TEXGEN_COORD \
if (pname < GL_TEXTURE_GEN_MODE || pname > GL_EYE_PLANE) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, pname); \
	return; \
}

void APIENTRY glTexGeni(GLenum coord, GLenum pname, GLint param)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(TexGeni, {}, { (int)coord, (int)pname, param });
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_TEXGEN_COORD;
	if (pname != GL_TEXTURE_GEN_MODE)
	{
		gl_set_error_a(GL_INVALID_ENUM, pname);
		return;
	}
	if (param < GL_EYE_LINEAR || param > GL_SPHERE_MAP)
	{
		gl_set_error_a(GL_INVALID_ENUM, param);
		return;
	}
	if (param == GL_SPHERE_MAP && (coord == GL_R || coord == GL_Q))
	{
		gl_set_error_a(GL_INVALID_ENUM, coord);
		return;
	}

	gl_state::texGen &tg = gs->texgen[coord - GL_S];
	tg.mode = param;
}
void APIENTRY glTexGenf(GLenum coord, GLenum pname, GLfloat param) { glTexGeni(coord, pname, (GLint)param); }
void APIENTRY glTexGend(GLenum coord, GLenum pname, GLdouble param) { glTexGeni(coord, pname, (GLint)param); }

static int gl_texGenv_size(GLenum pname)
{
	if (pname == GL_TEXTURE_GEN_MODE)
		return 1;
	else if (pname == GL_OBJECT_PLANE || pname == GL_EYE_PLANE)
		return 4;
	return 0;
}

template<typename T>
void APIENTRY gl_texGenv(GLenum coord, GLenum pname, const T *params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST_FV(TexGenfv, params, gl_texGenv_size(pname), {}, { (int)coord, (int)pname });
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_TEXGEN_V;
	if (pname == GL_TEXTURE_GEN_MODE
		&& (params[0] < GL_EYE_LINEAR || params[0] > GL_SPHERE_MAP))
	{
		gl_set_error_a(GL_INVALID_ENUM, (GLenum)params[0]);
		return;
	}
	if (pname == GL_TEXTURE_GEN_MODE
		&& params[0] == GL_SPHERE_MAP && (coord == GL_R || coord == GL_Q))
	{
		gl_set_error_a(GL_INVALID_ENUM, coord);
		return;
	}

	gl_state::texGen &tg = gs->texgen[coord - GL_S];

	if (pname == GL_TEXTURE_GEN_MODE)
		tg.mode = (GLenum)params[0];
	else if (pname == GL_OBJECT_PLANE)
		tg.object_plane = glm::vec4(params[0], params[1], params[2], params[3]);
	else if (pname == GL_EYE_PLANE)
		tg.eye_plane = glm::vec4(params[0], params[1], params[2], params[3]) * gs->get_inv_modelview();
}

void APIENTRY glTexGeniv(GLenum coord, GLenum pname, const GLint * params) { gl_texGenv(coord, pname, params); }
void APIENTRY glTexGenfv(GLenum coord, GLenum pname, const GLfloat * params) { gl_texGenv(coord, pname, params); }
void APIENTRY glTexGendv(GLenum coord, GLenum pname, const GLdouble * params) { gl_texGenv(coord, pname, params); }

template<typename T>
void gl_getTexGenv(GLenum coord, GLenum pname, T *params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_TEXGEN_V;

	gl_state::texGen &tg = gs->texgen[coord - GL_S];

	if (pname == GL_TEXTURE_GEN_MODE)
		*params = (T)tg.mode;
	else if (pname == GL_OBJECT_PLANE)
		copy_vals(params, &tg.object_plane.x, 4);
	else if (pname == GL_EYE_PLANE)
		copy_vals(params, &tg.eye_plane.x, 4);
}

void APIENTRY glGetTexGeniv(GLenum coord, GLenum pname, GLint *params)
{
	gl_getTexGenv(coord, pname, params);
}

void APIENTRY glGetTexGenfv(GLenum coord, GLenum pname, GLfloat *params)
{
	gl_getTexGenv(coord, pname, params);
}
