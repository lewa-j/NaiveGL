#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"
#include <glm/trigonometric.hpp>

glm::vec4 gl_state::get_vertex_color(const glm::vec4& vertex_view, const glm::vec4& color, glm::vec3 normal, bool front_face)
{
	if (!lighting.enabled)
		return glm::clamp(color, glm::vec4(0), glm::vec4(1));

	if (lighting.color_material && lighting.light_model_two_side)
		set_material_color(lighting.color_material_face, lighting.color_material_param, color, true);

	int ms = front_face ? 0 : 1;//material side
	const gl_state::lighting_t::material &m = lighting.materials[ms];

	glm::vec4 r = m.emission
		+ m.ambient * lighting.light_model_ambient;

	if (!lighting.enabled_lights)
	{
		r.a = m.diffuse.a;
		r = glm::clamp(r, glm::vec4(0), glm::vec4(1));
		return r;
	}

	const glm::vec4 p_eye(0, 0, 0, 1);
	if (!front_face)
		normal = -normal;

	for (int i = 0; i < gl_max_lights; i++)
	{
		if (!(lighting.enabled_lights & (1 << i)))
			continue;
		const gl_state::lighting_t::light &l = lighting.lights[i];

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
		glm::vec3 h = glm::normalize(lighting.light_model_local_viewer ? (VP - glm::normalize(glm::vec3(vertex_view))) : (VP + glm::vec3(0, 0, 1)));
		float NdotL = glm::max(0.0f, glm::dot(normal, VP));
		float f = (NdotL != 0) ? 1.0f : 0.0f;
		r += att * spot * (m.ambient * l.ambient
			+ NdotL * m.diffuse * l.diffuse
			+ f * glm::pow(glm::max(0.0f, glm::dot(normal, h)), m.shininess) * m.specular * l.specular);
	}

	r.a = lighting.materials[ms].diffuse.a;
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
		gs->lighting.materials[0].shininess = param;
	if (face != GL_FRONT)
		gs->lighting.materials[1].shininess = param;
}

void gl_state::set_material_color(GLenum face, GLenum pname, const glm::vec4 &param, bool force)
{
	if (!force && lighting.color_material
		&& (pname == lighting.color_material_param || (lighting.color_material_param == GL_AMBIENT_AND_DIFFUSE && (pname == GL_AMBIENT || pname == GL_DIFFUSE))))
	{
		if (face == lighting.color_material_face || lighting.color_material_face == GL_FRONT_AND_BACK)
			return;
	}

	if (face == GL_FRONT_AND_BACK)
	{
		set_material_color(GL_FRONT, pname, param, force);
		set_material_color(GL_BACK, pname, param, force);
		return;
	}

	if (pname == GL_AMBIENT_AND_DIFFUSE)
	{
		set_material_color(face, GL_AMBIENT, param, force);
		set_material_color(face, GL_DIFFUSE, param, force);
		return;
	}

	if (pname == GL_AMBIENT)
		lighting.materials[face == GL_FRONT ? 0 : 1].ambient = param;
	else if (pname == GL_DIFFUSE)
		lighting.materials[face == GL_FRONT ? 0 : 1].diffuse = param;
	else if (pname == GL_SPECULAR)
		lighting.materials[face == GL_FRONT ? 0 : 1].specular = param;
	else if (pname == GL_EMISSION)
		lighting.materials[face == GL_FRONT ? 0 : 1].emission = param;
}

void gl_state::update_color_material()
{
	set_material_color(lighting.color_material_face, lighting.color_material_param, current.color, true);
}

void APIENTRY glFrontFace(GLenum mode)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(FrontFace, {}, { (int)mode });
	VALIDATE_NOT_BEGIN_MODE;

	if (mode != GL_CW && mode != GL_CCW)
	{
		gl_set_error_a(GL_INVALID_ENUM, mode);
		return;
	}

	gs->polygon.front_face_ccw = (mode == GL_CCW);
}

void APIENTRY glShadeModel(GLenum mode)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(ShadeModel, {}, { (int)mode });
	VALIDATE_NOT_BEGIN_MODE;

	if (mode != GL_FLAT && mode != GL_SMOOTH)
	{
		gl_set_error_a(GL_INVALID_ENUM, mode);
		return;
	}

	gs->lighting.shade_model_flat = (mode == GL_FLAT);
}

#define VALIDATE_MAT_PNAME_V \
if ((pname < GL_EMISSION || pname > GL_COLOR_INDEXES) && (pname < GL_AMBIENT && pname > GL_SPECULAR))\
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
	WRITE_DISPLAY_LIST(ColorMaterial, {}, { (int)face, (int)mode });
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_FACE;

	if ((mode < GL_AMBIENT && mode > GL_SPECULAR) && mode != GL_EMISSION && mode != GL_AMBIENT_AND_DIFFUSE)
	{
		gl_set_error_a(GL_INVALID_ENUM, mode);
		return;
	}

	gs->lighting.color_material_face = face;
	gs->lighting.color_material_param = mode;

	if (gs->lighting.color_material)
		gs->update_color_material();
}

//glMaterial* can be called after glBegin

void APIENTRY glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(Material, { param }, { (int)face, (int)pname });
	VALIDATE_FACE;

	if (pname == GL_SHININESS)
		set_material_shininess(gs, face, param);
	else
		gl_set_error(GL_INVALID_ENUM);
}
void APIENTRY glMateriali(GLenum face, GLenum pname, GLint param) { glMaterialf(face, pname, (GLfloat)param); }

static int gl_materialv_size(GLenum pname)
{
	if (pname == GL_SHININESS)
		return 1;
	if (pname == GL_COLOR_INDEXES)
		return 3;
	if (pname == GL_EMISSION || pname == GL_AMBIENT_AND_DIFFUSE || (pname >= GL_AMBIENT && pname <= GL_SPECULAR))
		return 4;
	return 0;
}

void APIENTRY glMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;

	if (pname == GL_COLOR_INDEXES)
		return;

	WRITE_DISPLAY_LIST_FV(Materialv, params, gl_materialv_size(pname), {}, { (int)face, (int)pname });
	VALIDATE_FACE;
	VALIDATE_MAT_PNAME_V;

	if (pname == GL_SHININESS)
		set_material_shininess(gs, face, params[0]);
	else
		gs->set_material_color(face, pname, glm::vec4(params[0], params[1], params[2], params[3]));
}

void APIENTRY glMaterialiv(GLenum face, GLenum pname, const GLint *params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;

	if (pname == GL_COLOR_INDEXES)
		return;

	if (gs->display_list_begun)
	{
		gl_display_list_call call{ gl_display_list_call::tMaterialv, {}, {(int)face, (int)pname} };
		if (pname == GL_SHININESS)
			call.argsf[0] = (GLfloat)params[0];
		else if (pname == GL_EMISSION || pname == GL_AMBIENT_AND_DIFFUSE || (pname >= GL_AMBIENT && pname <= GL_SPECULAR))
		{
			call.argsf[0] = GLtof(params[0]);
			call.argsf[1] = GLtof(params[1]);
			call.argsf[2] = GLtof(params[2]);
			call.argsf[3] = GLtof(params[3]);
		}

		gs->display_list_indices[0].calls.push_back(call);
		if (!gs->display_list_execute)
			return;
	}

	VALIDATE_FACE;
	VALIDATE_MAT_PNAME_V;

	if (pname == GL_SHININESS)
		set_material_shininess(gs, face, (GLfloat)params[0]);
	else
		gs->set_material_color(face, pname, glm::vec4(GLtof(params[0]), GLtof(params[1]), GLtof(params[2]), GLtof(params[3])));
}

template<typename T>
void gl_getMaterialv(GLenum face, GLenum pname, T *params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (face != GL_FRONT && face != GL_BACK)
	{
		gl_set_error_a(GL_INVALID_ENUM, face);
		return;
	}
	VALIDATE_MAT_PNAME_V;

	if (pname == GL_COLOR_INDEXES)
		return;

	const gl_state::lighting_t::material &mat = gs->lighting.materials[face == GL_FRONT ? 0 : 1];

	if (pname == GL_SHININESS)
		copy_vals(params, &mat.shininess, 1);
	else if (pname == GL_AMBIENT)
		copy_color(params, &mat.ambient.x);
	else if (pname == GL_DIFFUSE)
		copy_color(params, &mat.diffuse.x);
	else if (pname == GL_SPECULAR)
		copy_color(params, &mat.specular.x);
	else if (pname == GL_EMISSION)
		copy_color(params, &mat.emission.x);
}

void APIENTRY glGetMaterialiv(GLenum face, GLenum pname, GLint *params)
{
	gl_getMaterialv(face, pname, params);
}

void APIENTRY glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
	gl_getMaterialv(face, pname, params);
}

static void gl_light_scalar(const char *func, gl_state::lighting_t::light &l, GLenum pname, GLfloat param)
{
	if (pname == GL_SPOT_EXPONENT)
	{
		if (param < 0 || param > 128)
		{
			gl_set_error_(GL_INVALID_VALUE, func);
			return;
		}
		l.spot_exponent = param;
	}
	else if (pname == GL_SPOT_CUTOFF)
	{
		if ((param < 0 || param > 90) && param != 180)
		{
			gl_set_error_(GL_INVALID_VALUE, func);
			return;
		}
		l.spot_cutoff = param;
	}
	else if (pname >= GL_CONSTANT_ATTENUATION && pname <= GL_QUADRATIC_ATTENUATION)
	{
		if (param < 0)
		{
			gl_set_error_(GL_INVALID_VALUE, func);
			return;
		}
		l.attenuation[pname - GL_CONSTANT_ATTENUATION] = param;
	}
}

void APIENTRY glLightf(GLenum light, GLenum pname, GLfloat param)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(Lightf, { param }, { (int)light, (int)pname });
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_LIGHT_NUM;
	VALIDATE_LIGHT_PNAME;

	gl_state::lighting_t::light &l = gs->lighting.lights[light - GL_LIGHT0];

	gl_light_scalar(__FUNCTION__, l, pname, param);
}

void APIENTRY glLighti(GLenum light, GLenum pname, GLint param) { glLightf(light, pname, (GLfloat)param); }

static void gl_light_color(gl_state::lighting_t::light &l, GLenum pname, glm::vec4 param)
{
	if (pname == GL_AMBIENT)
		l.ambient = param;
	else if (pname == GL_DIFFUSE)
		l.diffuse = param;
	else if (pname == GL_SPECULAR)
		l.specular = param;
}

static int gl_lightv_size(GLenum pname)
{
	if (pname >= GL_AMBIENT && pname <= GL_POSITION)
		return 4;
	if (pname == GL_SPOT_DIRECTION)
		return 3;
	if (pname >= GL_SPOT_EXPONENT && pname <= GL_QUADRATIC_ATTENUATION)
		return 1;
	return 0;
}

template<typename T>
void gl_lightv(gl_state *gs, GLenum light, GLenum pname, const T *params)
{
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_LIGHT_NUM;
	VALIDATE_LIGHT_PNAME_V;

	gl_state::lighting_t::light &l = gs->lighting.lights[light - GL_LIGHT0];

	if (pname >= GL_AMBIENT && pname <= GL_SPECULAR)
		gl_light_color(l, pname, glm::vec4(GLtof(params[0]), GLtof(params[1]), GLtof(params[2]), GLtof(params[3])));
	else if (pname == GL_POSITION)
		l.position = gs->get_modelview() * glm::vec4(params[0], params[1], params[2], params[3]);
	else if (pname == GL_SPOT_DIRECTION)
		l.spot_direction = glm::mat3(gs->get_modelview()) * glm::vec3(params[0], params[1], params[2]);
	else
		gl_light_scalar(__FUNCTION__, l, pname, (GLfloat)params[0]);
}

void APIENTRY glLightiv(GLenum light, GLenum pname, const GLint *params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST_IV(Lightiv, params, gl_lightv_size(pname), 2, {}, { (int)light, (int)pname });

	gl_lightv(gs, light, pname, params);
}

void APIENTRY glLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST_FV(Lightfv, params, gl_lightv_size(pname), {}, { (int)light, (int)pname });

	gl_lightv(gs, light, pname, params);
}

template<typename T>
static void gl_getLightv(GLenum light, GLenum pname, T *params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_LIGHT_NUM;
	VALIDATE_LIGHT_PNAME_V;

	gl_state::lighting_t::light &l = gs->lighting.lights[light - GL_LIGHT0];

	if (pname == GL_AMBIENT)
		copy_color(params, &l.ambient.x);
	else if (pname == GL_DIFFUSE)
		copy_color(params, &l.diffuse.x);
	else if (pname == GL_SPECULAR)
		copy_color(params, &l.specular.x);
	else if (pname == GL_POSITION)
		copy_vals(params, &l.position.x, 4);
	else if (pname == GL_SPOT_DIRECTION)
		copy_vals(params, &l.spot_direction.x, 3);
	else if (pname == GL_SPOT_EXPONENT)
		copy_vals(params, &l.spot_exponent, 1);
	else if (pname == GL_SPOT_CUTOFF)
		copy_vals(params, &l.spot_cutoff, 1);
	else if (pname >= GL_CONSTANT_ATTENUATION && pname <= GL_QUADRATIC_ATTENUATION)
		copy_vals(params, &l.attenuation[pname - GL_CONSTANT_ATTENUATION], 1);
}

void APIENTRY glGetLightiv(GLenum light, GLenum pname, GLint *params)
{
	gl_getLightv(light, pname, params);
}

void APIENTRY glGetLightfv(GLenum light, GLenum pname, GLfloat *params)
{
	gl_getLightv(light, pname, params);
}

void APIENTRY glLightModeli(GLenum pname, GLint param)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(LightModeli, {}, { (int)pname, param });
	VALIDATE_NOT_BEGIN_MODE;
	if (pname != GL_LIGHT_MODEL_LOCAL_VIEWER && pname != GL_LIGHT_MODEL_TWO_SIDE)
	{
		gl_set_error_a(GL_INVALID_ENUM, pname);
		return;
	}

	if (pname == GL_LIGHT_MODEL_LOCAL_VIEWER)
		gs->lighting.light_model_local_viewer = (param != 0);
	else if (pname == GL_LIGHT_MODEL_TWO_SIDE)
		gs->lighting.light_model_two_side = (param != 0);
}

void APIENTRY glLightModelf(GLenum pname, GLfloat param) { glLightModeli(pname, to_int(param)); }

static int gl_lightModelv_size(GLenum pname)
{
	if (pname == GL_LIGHT_MODEL_LOCAL_VIEWER || pname == GL_LIGHT_MODEL_TWO_SIDE)
		return 1;
	if (pname == GL_LIGHT_MODEL_AMBIENT)
		return 4;
	return 0;
}

template<typename T>
void APIENTRY gl_lightModelv(gl_state *gs, GLenum pname, const T *params)
{
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_LIGHT_MODEL_PNAME_V;

	if (pname == GL_LIGHT_MODEL_AMBIENT)
		gs->lighting.light_model_ambient = glm::vec4(GLtof(params[0]), GLtof(params[1]), GLtof(params[2]), GLtof(params[3]));
	else if (pname == GL_LIGHT_MODEL_LOCAL_VIEWER)
		gs->lighting.light_model_local_viewer = (params[0] != 0);
	else if (pname == GL_LIGHT_MODEL_TWO_SIDE)
		gs->lighting.light_model_two_side = (params[0] != 0);
}

void APIENTRY glLightModelfv(GLenum pname, const GLfloat *params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST_FV(LightModelfv, params, gl_lightModelv_size(pname), {}, { (int)pname });

	gl_lightModelv(gs, pname, params);
}

void APIENTRY glLightModeliv(GLenum pname, const GLint *params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST_IV(LightModeliv, params, gl_lightModelv_size(pname), 1, {}, { (int)pname });

	gl_lightModelv(gs, pname, params);
}
