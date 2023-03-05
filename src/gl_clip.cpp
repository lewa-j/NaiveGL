#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"

void APIENTRY glClipPlane(GLenum plane, const GLdouble *equation)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (plane < GL_CLIP_PLANE0 || plane >= (GL_CLIP_PLANE0 + gl_max_user_clip_planes))
	{
		gl_set_error_a(GL_INVALID_ENUM, plane);
		return;
	}

	gs->clipplanes[plane - GL_CLIP_PLANE0] = glm::vec4(equation[0], equation[1], equation[2], equation[3]) * gs->get_inv_modelview();
}
