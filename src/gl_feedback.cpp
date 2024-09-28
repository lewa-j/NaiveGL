#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"

EXPORT void APIENTRY glFeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (type < GL_2D || type > GL_4D_COLOR_TEXTURE)
	{
		gl_set_error_a(GL_INVALID_ENUM, type);
		return;
	}

	if (size < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	if (gs->render_mode == GL_FEEDBACK)
	{
		gl_set_error(GL_INVALID_OPERATION);
		return;
	}

	gs->feedback.buffer_type = type;
	gs->feedback.buffer = buffer;
	gs->feedback.buffer_size = size;
}

EXPORT void APIENTRY glPassThrough(GLfloat token)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(PassThrough, { token });
	VALIDATE_NOT_BEGIN_MODE;

	if (gs->render_mode != GL_FEEDBACK)
		return;

	gl_feedback_write(*gs, GL_PASS_THROUGH_TOKEN);
	gl_feedback_write(*gs, token);
}

void gl_feedback_write(gl_state &st, float f)
{
	if (!st.feedback.buffer)
		return;

	if (st.feedback_overflow)
		return;

	*st.feedback_array_pos = f;
	st.feedback_array_pos++;

	st.feedback_overflow = st.feedback_array_pos == st.feedback.buffer + st.feedback.buffer_size;
}

void gl_feedback_write_vertex(gl_state &st, glm::vec4 pos, glm::vec4 color, glm::vec4 tex_coord)
{
	gl_feedback_write(st, pos.x);
	gl_feedback_write(st, pos.y);
	if (st.feedback.buffer_type == GL_2D)
		return;
	gl_feedback_write(st, pos.z);
	if (st.feedback.buffer_type == GL_3D)
		return;
	if (st.feedback.buffer_type == GL_4D_COLOR_TEXTURE)
		gl_feedback_write(st, pos.w);

	// index color mode
	// write 1 float

	// rgba color mode
	gl_feedback_write(st, color.r);
	gl_feedback_write(st, color.g);
	gl_feedback_write(st, color.b);
	gl_feedback_write(st, color.a);
	if (st.feedback.buffer_type == GL_3D_COLOR)
		return;
	
	gl_feedback_write(st, tex_coord.s);
	gl_feedback_write(st, tex_coord.t);
	gl_feedback_write(st, tex_coord.r);
	gl_feedback_write(st, tex_coord.q);
}

void gl_feedback_write_vertex(gl_state &st, const gl_processed_vertex &v)
{
	glm::vec3 win = st.get_window_coords(glm::vec3(v.clip) / v.clip.w);
	gl_feedback_write_vertex(st, { win, v.clip.w }, v.color, v.tex_coord);
}