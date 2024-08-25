#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"
#include <glm/common.hpp>

void gl_state::set_viewport(int x, int y, int w, int h)
{
	viewport.width = w;
	viewport.height = h;
#if NAGL_FLIP_VIEWPORT_Y
	viewport.height *= -1;
#endif
	viewport.center_x = x + w / 2;
	viewport.center_y = y + h / 2;
}

glm::vec3 gl_state::get_window_coords(glm::vec3 device_coords)
{
	return device_coords * glm::vec3(viewport.width, viewport.height, viewport.dfar - viewport.dnear) * 0.5f
		+ glm::vec3(viewport.center_x, viewport.center_y, (viewport.dnear + viewport.dfar) * 0.5f);
}

void APIENTRY glDepthRange(GLdouble n, GLdouble f)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(DepthRange, { (float)n, (float)f });
	VALIDATE_NOT_BEGIN_MODE;

	gs->viewport.dnear = (float)glm::clamp(n, 0.0, 1.0);
	gs->viewport.dfar = (float)glm::clamp(f, 0.0, 1.0);
}

void APIENTRY glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(Viewport, {}, { x,y,width,height });
	VALIDATE_NOT_BEGIN_MODE;

	if (width < 0 || height < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	width = glm::min(width, gl_max_viewport_dims[0]);
	height = glm::min(height, gl_max_viewport_dims[1]);

	gs->set_viewport(x,y,width,height);
}
