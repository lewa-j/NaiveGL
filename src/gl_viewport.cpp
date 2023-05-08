#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"
#include <glm/common.hpp>

glm::vec3 gl_state::get_window_coords(glm::vec3 device_coords)
{
	return device_coords * glm::vec3(viewport.width, viewport.height, viewport.dfar - viewport.dnear) * 0.5f
		+ glm::vec3(viewport.center_x, viewport.center_y, (viewport.dnear + viewport.dfar) * 0.5f);
}

void APIENTRY glDepthRange(GLdouble n, GLdouble f)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	gs->viewport.dnear = (float)glm::clamp(n, 0.0, 1.0);
	gs->viewport.dfar = (float)glm::clamp(f, 0.0, 1.0);
}

void APIENTRY glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (width < 0 || height < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	//TODO clamp by max
	gs->viewport.width = width;
	gs->viewport.height = height;
	gs->viewport.center_x = x + width / 2;
	gs->viewport.center_y = y + height / 2;
}
