#include "ViewportController.h"

ViewportController::ViewportController(OcctViewport* viewport)
  : m_viewport(viewport)
{
}

void
ViewportController::onResize()
{
	if (m_viewport && m_viewport->getView())
	{
		m_viewport->getView()->MustBeResized();
	}
}

void
ViewportController::onMousePressEvent(QMouseEvent* e)
{
	if (!m_viewport || !m_viewport->getView())
	{
		return;
	}

	if (!m_rotationEnabled && e->button() == Qt::LeftButton)
	{
		return;
	}

	// Map Qt button enum to OCCT enum
	Aspect_VKeyMouse btn = Aspect_VKeyMouse_NONE;
	if (e->button() == Qt::LeftButton)
	{
		btn = Aspect_VKeyMouse_LeftButton;
	}
	else if (e->button() == Qt::RightButton)
	{
		btn = Aspect_VKeyMouse_RightButton;
	}
	else if (e->button() == Qt::MiddleButton)
	{
		btn = Aspect_VKeyMouse_MiddleButton;
	}

	Graphic3d_Vec2i pos(e->position().x(), e->position().y());
	PressMouseButton(pos, btn, Aspect_VKeyFlags_NONE, false);
	FlushViewEvents(m_viewport->getContext(), m_viewport->getView(), true);
}

void
ViewportController::onMouseReleaseEvent(QMouseEvent* e)
{
	if (!m_viewport || !m_viewport->getView())
	{
		return;
	}

	if (!m_rotationEnabled && e->button() == Qt::LeftButton)
	{
		return;
	}

	// Map Qt button enum to OCCT enum
	Aspect_VKeyMouse btn = Aspect_VKeyMouse_NONE;
	if (e->button() == Qt::LeftButton)
	{
		btn = Aspect_VKeyMouse_LeftButton;
	}
	else if (e->button() == Qt::RightButton)
	{
		btn = Aspect_VKeyMouse_RightButton;
	}
	else if (e->button() == Qt::MiddleButton)
	{
		btn = Aspect_VKeyMouse_MiddleButton;
	}

	Graphic3d_Vec2i pos(e->position().x(), e->position().y());
	ReleaseMouseButton(pos, btn, Aspect_VKeyFlags_NONE, false);
	FlushViewEvents(m_viewport->getContext(), m_viewport->getView(), true);
}

void
ViewportController::onMouseMoveEvent(QMouseEvent* e)
{
	if (!m_viewport || !m_viewport->getView())
	{
		return;
	}

	Graphic3d_Vec2i pos(e->position().x(), e->position().y());
	// Aggregate all pressed buttons during motion
	Aspect_VKeyMouse buttons = Aspect_VKeyMouse_NONE;

	if (e->buttons() & Qt::LeftButton)
	{
		buttons |= Aspect_VKeyMouse_LeftButton;
	}
	if (e->buttons() & Qt::RightButton)
	{
		buttons |= Aspect_VKeyMouse_RightButton;
	}
	if (e->buttons() & Qt::MiddleButton)
	{
		buttons |= Aspect_VKeyMouse_MiddleButton;
	}

	UpdateMousePosition(pos, buttons, Aspect_VKeyFlags_NONE, false);
	FlushViewEvents(m_viewport->getContext(), m_viewport->getView(), true);
}

void
ViewportController::onWheelEvent(QWheelEvent* e)
{
	if (!m_viewport->getView())
	{
		return;
	}

	Graphic3d_Vec2i pos(e->position().x(), e->position().y());
	// Convert wheel delta to normalized zoom speed (angleDelta ~ 120 per tick)
	double delta = e->angleDelta().y() / 8.0 / 15.0;

	UpdateMouseScroll(Aspect_ScrollDelta(pos, delta, Aspect_VKeyFlags_NONE));
	FlushViewEvents(m_viewport->getContext(), m_viewport->getView(), true);
}

void
ViewportController::onKeyEvent(QKeyEvent* e)
{
	switch (e->key())
	{
		case Qt::Key_F:
			if (m_viewport->getContext()->NbSelected() > 0)
			{
				m_viewport->fitSelected();
			}
			else
			{
				m_viewport->fitAll();
			}
			break;

		case Qt::Key_S:
			m_viewport->setShadingMode(false);
			break;

		case Qt::Key_W:
			m_viewport->setShadingMode(true);
			break;

		case Qt::Key_Backspace:
			m_viewport->setViewPreset(V3d_XposYnegZpos);
			break;
		case Qt::Key_T:
			m_viewport->setViewPreset(V3d_TypeOfOrientation_Zup_Top);
			break;
		case Qt::Key_B:
			m_viewport->setViewPreset(V3d_TypeOfOrientation_Zup_Bottom);
			break;
		case Qt::Key_L:
			m_viewport->setViewPreset(V3d_TypeOfOrientation_Zup_Left);
			break;
		case Qt::Key_R:
			m_viewport->setViewPreset(V3d_TypeOfOrientation_Zup_Right);
			break;
	}
}

void
ViewportController::setRotationEnabled(bool enabled)
{
	m_rotationEnabled = enabled;
}
