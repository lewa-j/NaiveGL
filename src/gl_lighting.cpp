#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"
#include <glm/trigonometric.hpp>

glm::vec4 gl_state::get_vertex_color(const glm::vec4 &vertex_object, const glm::vec4 &vertex_view, bool front_face)
{
	if (!lighting_enabled)
		return current_color;

	int ms = front_face ? 0 : 1;//material side
	const gl_state::material &m = materials[ms];

	glm::vec4 r = m.emission
		+ m.ambient * light_model_ambient;

	if (!enabled_lights)
	{
		r.a = m.diffuse.a;
		r = glm::clamp(r, glm::vec4(0), glm::vec4(1));
		return r;
	}

	const glm::vec4 p_eye(0, 0, 0, 1);
	glm::vec3 normal = get_eye_normal();
	if (!front_face)
		normal = -normal;

	for (int i = 0; i < gl_max_lights; i++)
	{
		if (!(enabled_lights & (1 << i)))
			continue;
		const gl_state::light &l = lights[i];

		float att = 1.0f;
		glm::vec3 VP = glm::vec3(l.position);
		if (l.position.w != 0)
		{
			float lVP = glm::length(glm::vec3(l.position - vertex_view));
			att = 1 / (l.attenuation[0] + l.attenuation[1] * lVP + l.attenuation[2] * lVP * lVP);
			VP = glm::vec3(l.position - vertex_view);
		}
		VP = glm::normalize(VP);
		float spot = 1.0;
		if (l.spot_cutoff != 180.0f)
		{
			float spot_dot = glm::max(0.0f, glm::dot(-VP, glm::normalize(l.spot_direction)));
			float c = cosf(glm::radians(l.spot_cutoff));
			spot = spot_dot < c ? 0.0f : pow(spot_dot, l.spot_exponent);
		}
		glm::vec3 h = glm::normalize(light_model_local_viewer ? (VP - glm::normalize(glm::vec3(vertex_view))) : (VP + glm::vec3(0, 0, 1)));
		float NdotL = glm::max(0.0f, glm::dot(normal, VP));
		float f = (NdotL != 0) ? 1 : 0;
		r += att * spot * (m.ambient * l.ambient
			+ NdotL * m.diffuse * l.diffuse
			+ f * glm::pow(glm::max(0.0f, glm::dot(normal, h)), m.shininess) * m.specular * l.specular);
	}

	r.a = materials[ms].diffuse.a;
	r = glm::clamp(r, glm::vec4(0), glm::vec4(1));

	return r;
}

static void set_material_shininess(gl_state *gs, GLenum face, GLfloat param)
{
	if (param < 0 || param > 128)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}
	if (face != GL_BACK)
		gs->materials[0].shininess = param;
	if (face != GL_FRONT)
		gs->materials[1].shininess = param;
}

static void set_material_color(gl_state *gs, GLenum face, GLenum pname, const glm::vec4 &param, bool force = false)
{
	if (!force && gs->color_material
		&& (pname == gs->color_material_mode || (gs->color_material_mode == GL_AMBIENT_AND_DIFFUSE && (pname == GL_AMBIENT || pname == GL_DIFFUSE))))
	{
		if (face == gs->color_material_face || gs->color_material_face == GL_FRONT_AND_BACK)
			return;
	}

	if (face == GL_FRONT_AND_BACK)
	{
		set_material_color(gs, GL_FRONT, pname, param, force);
		set_material_color(gs, GL_BACK, pname, param, force);
		return;
	}

	if (pname == GL_AMBIENT_AND_DIFFUSE)
	{
		set_material_color(gs, face, GL_AMBIENT, param, force);
		set_material_color(gs, face, GL_DIFFUSE, param, force);
		return;
	}

	if (pname == GL_AMBIENT)
		gs->materials[face == GL_FRONT ? 0 : 1].ambient = param;
	else if (pname == GL_DIFFUSE)
		gs->materials[face == GL_FRONT ? 0 : 1].diffuse = param;
	else if (pname == GL_SPECULAR)
		gs->materials[face == GL_FRONT ? 0 : 1].specular = param;
	else if (pname == GL_EMISSION)
		gs->materials[face == GL_FRONT ? 0 : 1].emission = param;
}

void gl_state::update_color_material()
{
	if (!color_material)
		return;

	set_material_color(this, color_material_face, color_material_mode, current_color, true);
}

void APIENTRY glFrontFace(GLenum mode)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (mode != GL_CW && mode != GL_CCW)
	{
		gl_set_error_a(GL_INVALID_ENUM, mode);
		return;
	}

	gs->front_face_ccw = (mode == GL_CCW);
}

void APIENTRY glShadeModel(GLenum mode)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (mode != GL_FLAT && mode != GL_SMOOTH)
	{
		gl_set_error_a(GL_INVALID_ENUM, mode);
		return;
	}

	gs->shade_model_flat = (mode == GL_FLAT);
}

#define VALIDATE_MAT_PNAME \
if (pname != GL_SHININESS)\
{\
	gl_set_error(GL_INVALID_ENUM);\
	return;\
}

#define VALIDATE_MAT_PNAME_V \
if ((pname < GL_EMISSION || pname > GL_AMBIENT_AND_DIFFUSE) && (pname < GL_AMBIENT && pname > GL_SPECULAR))\
{\
	gl_set_error_a(GL_INVALID_ENUM, pname);\
	return;\
}

#define VALIDATE_LIGHT_NUM \
if (light < GL_LIGHT0 || light >= GL_LIGHT0 + gl_max_lights)\
{\
	gl_set_error_a(GL_INVALID_ENUM, light);\
	return;\
}

#define VALIDATE_LIGHT_PNAME \
if (pname < GL_SPOT_EXPONENT || pname > GL_QUADRATIC_ATTENUATION)\
{\
	gl_set_error_a(GL_INVALID_ENUM, pname);\
	return;\
}

#define VALIDATE_LIGHT_PNAME_V \
if (pname < GL_AMBIENT || pname > GL_QUADRATIC_ATTENUATION)\
{\
	gl_set_error_a(GL_INVALID_ENUM, pname);\
	return;\
}

#define VALIDATE_LIGHT_MODEL_PNAME_V \
if (pname != GL_LIGHT_MODEL_LOCAL_VIEWER && pname != GL_LIGHT_MODEL_TWO_SIDE && pname != GL_LIGHT_MODEL_AMBIENT)\
{\
	gl_set_error_a(GL_INVALID_ENUM, pname);\
	return;\
}

void APIENTRY glColorMaterial(GLenum face, GLenum mode)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_FACE;

	if ((mode < GL_AMBIENT && mode > GL_SPECULAR) && mode != GL_EMISSION && mode != GL_AMBIENT_AND_DIFFUSE)
	{
		gl_set_error_a(GL_INVALID_ENUM, mode);
		return;
	}

	gs->color_material_face = face;
	gs->color_material_mode = mode;

	if (gs->color_material)
		gs->update_color_material();
}

//glMaterial* can be called after glBegin

void APIENTRY glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_FACE;
	VALIDATE_MAT_PNAME;

	if (pname == GL_SHININESS)
		set_material_shininess(gs, face, param);
}
void APIENTRY glMateriali(GLenum face, GLenum pname, GLint param) { glMaterialf(face, pname, param); }

void APIENTRY glMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_FACE;
	VALIDATE_MAT_PNAME_V;

	if (pname == GL_SHININESS)
		set_material_shininess(gs, face, params[0]);
	else
		set_material_color(gs, face, pname, glm::vec4(params[0], params[1], params[2], params[3]));
}

void APIENTRY glMaterialiv(GLenum face, GLenum pname, const GLint *params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_FACE;
	VALIDATE_MAT_PNAME_V;

	if (pname == GL_SHININESS)
		set_material_shininess(gs, face, params[0]);
	else
		set_material_color(gs, face, pname, glm::vec4(GLtof(params[0]), GLtof(params[1]), GLtof(params[2]), GLtof(params[3])));
}

void APIENTRY glLightf(GLenum light, GLenum pname, GLfloat param)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_LIGHT_NUM;
	VALIDATE_LIGHT_PNAME;

	gl_state::light &l = gs->lights[light - GL_LIGHT0];

	if (pname == GL_SPOT_EXPONENT)
	{
		if (param < 0 || param > 128)
		{
			gl_set_error(GL_INVALID_VALUE);
			return;
		}
		l.spot_exponent = param;
	}
	else if (pname == GL_SPOT_CUTOFF)
	{
		if ((param < 0 || param > 90) && param != 180)
		{
			gl_set_error(GL_INVALID_VALUE);
			return;
		}
		l.spot_cutoff = param;
	}
	else if (pname >= GL_CONSTANT_ATTENUATION && pname <= GL_QUADRATIC_ATTENUATION)
	{
		if (param < 0)
		{
			gl_set_error(GL_INVALID_VALUE);
			return;
		}
		l.attenuation[pname - GL_CONSTANT_ATTENUATION] = param;
	}
}
void APIENTRY glLighti(GLenum light, GLenum pname, GLint param) { glLightf(light, pname, param); }

static void gl_light_color(gl_state::light &l, GLenum pname, glm::vec4 param)
{
	if (pname == GL_AMBIENT)
		l.ambient = param;
	else if (pname == GL_DIFFUSE)
		l.diffuse = param;
	else if (pname == GL_SPECULAR)
		l.specular = param;
}

template<typename T>
void gl_lightv(GLenum light, GLenum pname, const T *params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_LIGHT_NUM;
	VALIDATE_LIGHT_PNAME_V;

	gl_state::light &l = gs->lights[light - GL_LIGHT0];

	if (pname >= GL_AMBIENT && pname <= GL_SPECULAR)
		gl_light_color(l, pname, glm::vec4(GLtof(params[0]), GLtof(params[1]), GLtof(params[2]), GLtof(params[3])));
	else if (pname == GL_POSITION)
		l.position = gs->get_modelview() * glm::vec4(params[0], params[1], params[2], params[3]);
	else if (pname == GL_SPOT_DIRECTION)
		l.spot_direction = glm::mat3(gs->get_modelview()) * glm::vec3(params[0], params[1], params[2]);
	else
		glLightf(light, pname, params[0]);
}
void APIENTRY glLightfv(GLenum light, GLenum pname, const GLfloat *params) { gl_lightv(light, pname, params); }
void APIENTRY glLightiv(GLenum light, GLenum pname, const GLint *params) { gl_lightv(light, pname, params); }

void APIENTRY glLightModelf(GLenum pname, GLfloat param)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (pname != GL_LIGHT_MODEL_LOCAL_VIEWER && pname != GL_LIGHT_MODEL_TWO_SIDE)
	{
		gl_set_error_a(GL_INVALID_ENUM, pname);
		return;
	}

	if (pname == GL_LIGHT_MODEL_LOCAL_VIEWER)
		gs->light_model_local_viewer = (param != 0);
	else if (pname == GL_LIGHT_MODEL_TWO_SIDE)
		gs->light_model_two_side = (param != 0);
}
void APIENTRY glLightModeli(GLenum pname, GLint param) { glLightModelf(pname, param); }

template<typename T>
void APIENTRY gl_lightModelv(GLenum pname, const T *params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_LIGHT_MODEL_PNAME_V;

	if (pname == GL_LIGHT_MODEL_AMBIENT)
		gs->light_model_ambient = glm::vec4(GLtof(params[0]), GLtof(params[1]), GLtof(params[2]), GLtof(params[3]));
	else if (pname == GL_LIGHT_MODEL_LOCAL_VIEWER)
		gs->light_model_local_viewer = (params[0] != 0);
	else if (pname == GL_LIGHT_MODEL_TWO_SIDE)
		gs->light_model_two_side = (params[0] != 0);
}
void APIENTRY glLightModelfv(GLenum pname, const GLfloat *params) { gl_lightModelv(pname, params); }
void APIENTRY glLightModeliv(GLenum pname, const GLint *params) { gl_lightModelv(pname, params); }
