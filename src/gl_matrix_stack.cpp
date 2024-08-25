#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"
#include <glm/gtc/matrix_transform.hpp>
#include <string.h>

const glm::mat4 &gl_state::get_modelview()
{
	return modelview_stack[modelview_sp];
}

glm::mat4 gl_state::get_inv_modelview()
{
	return glm::inverse(modelview_stack[modelview_sp]);
}

const glm::mat4 &gl_state::get_projection()
{
	return projection_stack[projection_sp];
}

const glm::mat4 &gl_state::get_mtx_texture()
{
	return texture_mtx_stack[texture_mtx_sp];
}

static int &get_current_mtx_sp(gl_state *gs)
{
	if (gs->matrix_mode == GL_MODELVIEW)
		return gs->modelview_sp;
	if (gs->matrix_mode == GL_PROJECTION)
		return gs->projection_sp;
	return gs->texture_mtx_sp;
}

static int get_current_mtx_max(gl_state *gs)
{
	if (gs->matrix_mode == GL_MODELVIEW)
		return gl_max_viewmodel_mtx;
	if (gs->matrix_mode == GL_PROJECTION)
		return gl_max_projection_mtx;
	return gl_max_texture_mtx;
}

static glm::mat4 &get_current_mtx(gl_state *gs)
{
	if (gs->matrix_mode == GL_MODELVIEW)
		return gs->modelview_stack[gs->modelview_sp];
	if (gs->matrix_mode == GL_PROJECTION)
		return gs->projection_stack[gs->projection_sp];
	return gs->texture_mtx_stack[gs->texture_mtx_sp];
}

void APIENTRY glMatrixMode(GLenum mode)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(MatrixMode, {}, { (int)mode });
	VALIDATE_NOT_BEGIN_MODE;

	if (mode < GL_MODELVIEW || mode > GL_TEXTURE)
	{
		gl_set_error_a(GL_INVALID_ENUM, mode);
		return;
	}

	gs->matrix_mode = mode;
}

void APIENTRY glLoadMatrixd(const GLdouble *m)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;

	if (gs->display_list_begun)
	{
		gl_display_list_call call{ gl_display_list_call::tLoadMatrix };
		glm::mat4 fm{ *(glm::dmat4 *)m };
		memcpy(call.argsf, &fm[0].x, 16 * sizeof(float));
		gs->display_list_indices[0].calls.push_back(call);
		if (!gs->display_list_execute)
			return;
	}

	VALIDATE_NOT_BEGIN_MODE;
	glm::mat4 &dst = get_current_mtx(gs);
	dst = glm::mat4(*(glm::dmat4 *)m);
}

void APIENTRY glLoadMatrixf(const GLfloat *m)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;

	if (gs->display_list_begun)
	{
		gl_display_list_call call{ gl_display_list_call::tLoadMatrix };
		memcpy(call.argsf, m, 16 * sizeof(float));
		gs->display_list_indices[0].calls.push_back(call);
		if (!gs->display_list_execute)
			return;
	}

	VALIDATE_NOT_BEGIN_MODE;
	memcpy(&get_current_mtx(gs), m, 16 * sizeof(float));
}

void APIENTRY glMultMatrixd(const GLdouble *m)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;

	if (gs->display_list_begun)
	{
		gl_display_list_call call{ gl_display_list_call::tMultMatrix };
		glm::mat4 fm{ *(glm::dmat4 *)m };
		memcpy(call.argsf, &fm[0].x, 16 * sizeof(float));
		gs->display_list_indices[0].calls.push_back(call);
		if (!gs->display_list_execute)
			return;
	}

	VALIDATE_NOT_BEGIN_MODE;
	glm::mat4 &dst = get_current_mtx(gs);
	dst = dst * glm::mat4(*(glm::dmat4 *)m);
}

void APIENTRY glMultMatrixf(const GLfloat *m)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (gs->display_list_begun)
	{
		gl_display_list_call call{ gl_display_list_call::tMultMatrix };
		memcpy(call.argsf, m, 16 * sizeof(float));
		gs->display_list_indices[0].calls.push_back(call);
		if (!gs->display_list_execute)
			return;
	}

	glm::mat4 &dst = get_current_mtx(gs);
	dst = dst * (*(glm::mat4 *)m);
}

void APIENTRY glLoadIdentity()
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(LoadIdentity);
	VALIDATE_NOT_BEGIN_MODE;

	get_current_mtx(gs) = glm::mat4(1);
}

void APIENTRY glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(Rotate, { angle,x,y,z });
	VALIDATE_NOT_BEGIN_MODE;

	glm::mat4 &dst = get_current_mtx(gs);
	dst = glm::rotate(dst, glm::radians(angle), glm::vec3(x, y, z));
}

void APIENTRY glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(Translate, { x,y,z });
	VALIDATE_NOT_BEGIN_MODE;

	glm::mat4 &dst = get_current_mtx(gs);
	dst = glm::translate(dst, glm::vec3(x, y, z));
}

void APIENTRY glScalef(GLfloat x, GLfloat y, GLfloat z)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(Scale, { x,y,z });
	VALIDATE_NOT_BEGIN_MODE;

	glm::mat4 &dst = get_current_mtx(gs);
	dst = glm::scale(dst, glm::vec3(x, y, z));
}

void APIENTRY glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z) { glRotatef((GLfloat)angle, (GLfloat)x, (GLfloat)y, (GLfloat)z); }
void APIENTRY glTranslated(GLdouble x, GLdouble y, GLdouble z) { glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)z); }
void APIENTRY glScaled(GLdouble x, GLdouble y, GLdouble z) { glScalef((GLfloat)x, (GLfloat)y, (GLfloat)z); }

void APIENTRY glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(Frustum, { (float)left, (float)right, (float)bottom, (float)top, (float)zNear, (float)zFar });
	VALIDATE_NOT_BEGIN_MODE;
	if (zNear <= 0 || zFar <= 0 || left == right || bottom == top || zNear == zFar)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	glm::mat4 &dst = get_current_mtx(gs);
	dst = dst * glm::frustum((float)left, (float)right, (float)bottom, (float)top, (float)zNear, (float)zFar);
}

void APIENTRY glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(Ortho, { (float)left, (float)right, (float)bottom, (float)top, (float)zNear, (float)zFar });
	VALIDATE_NOT_BEGIN_MODE;
	if (left == right || bottom == top || zNear == zFar)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	glm::mat4 &dst = get_current_mtx(gs);
	dst = dst * glm::ortho((float)left, (float)right, (float)bottom, (float)top, (float)zNear, (float)zFar);
}

void APIENTRY glPushMatrix()
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(PushMatrix);
	VALIDATE_NOT_BEGIN_MODE;
	int &sp = get_current_mtx_sp(gs);
	int max = get_current_mtx_max(gs);
	if (sp + 1 >= max)
	{
		gl_set_error(GL_STACK_OVERFLOW);
		return;
	}
	float *m = &get_current_mtx(gs)[0][0];
	memcpy(m + 16, m, 16 * sizeof(float));
	sp++;
}

void APIENTRY glPopMatrix()
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(PopMatrix);
	VALIDATE_NOT_BEGIN_MODE;
	int &sp = get_current_mtx_sp(gs);
	if (sp - 1 < 0)
	{
		gl_set_error(GL_STACK_UNDERFLOW);
		return;
	}
	sp--;
}
