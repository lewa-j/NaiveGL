#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"

void APIENTRY glInitNames(void)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (gs->render_mode != GL_SELECT)
		return;

	if (gs->select_hit)
		gl_flush_selection_hit_record(*gs);
	gs->select_name_sp = 0;
}

void APIENTRY glPopName(void)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (gs->render_mode != GL_SELECT)
		return;

	if (gs->select_name_sp == 0)
	{
		gl_set_error(GL_STACK_UNDERFLOW);
		return;
	}

	if (gs->select_hit)
		gl_flush_selection_hit_record(*gs);
	gs->select_name_sp--;
}

void APIENTRY glPushName(GLuint name)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (gs->render_mode != GL_SELECT)
		return;

	if (gs->select_name_sp >= gl_max_name_stack_depth)
	{
		gl_set_error(GL_STACK_OVERFLOW);
		return;
	}

	if (gs->select_hit)
		gl_flush_selection_hit_record(*gs);
	gs->select_name_stack[gs->select_name_sp] = name;
	gs->select_name_sp++;
}

void APIENTRY glLoadName(GLuint name)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (gs->render_mode != GL_SELECT)
		return;

	if (gs->select_name_sp == 0)
	{
		gl_set_error(GL_INVALID_OPERATION);
		return;
	}

	if (gs->select_hit)
		gl_flush_selection_hit_record(*gs);
	gs->select_name_stack[gs->select_name_sp - 1] = name;
}

void APIENTRY glSelectBuffer(GLsizei size, GLuint *buffer)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (size <= 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	if (gs->render_mode == GL_SELECT)
	{
		gl_set_error(GL_INVALID_OPERATION);
		return;
	}

	gs->selection_array = buffer;
	gs->selection_array_max_size = size;
}

void gl_add_selection_depth(gl_state &st, float z)
{
	GLuint d = (GLuint)roundf(z * UINT_MAX);
	st.select_min_depth = glm::min(st.select_min_depth, d);
	st.select_max_depth = glm::max(st.select_max_depth, d);
}

void gl_sel_write(gl_state &st, GLuint val)
{
	if (st.selection_array_pos >= st.selection_array + st.selection_array_max_size)
	{
		st.select_overflow = true;
		return;
	}

	*st.selection_array_pos = val;
	st.selection_array_pos++;
}

void gl_flush_selection_hit_record(gl_state &st)
{
	if (!st.selection_array)
		return;

	gl_sel_write(st, st.select_name_sp);
	gl_sel_write(st, st.select_min_depth);
	gl_sel_write(st, st.select_max_depth);
	for (int i = 0; i < st.select_name_sp; i++)
		gl_sel_write(st, st.select_name_stack[i]);

	st.select_hit_records++;

	st.select_min_depth = UINT_MAX;
	st.select_max_depth = 0;
	st.select_hit = false;
}
