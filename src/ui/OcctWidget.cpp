#include "OcctWidget.h"
#include "app/ViewportController.h"

OcctWidget::OcctWidget(QWidget* parent)
  : QWidget(parent)
{
	setAttribute(Qt::WA_NativeWindow);
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NoSystemBackground);
	setAutoFillBackground(false);

	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);
}

void
OcctWidget::resizeEvent(QResizeEvent* e)
{
	if (m_controller)
	{
		m_controller->onResize();
	}
	QWidget::resizeEvent(e);
}

void
OcctWidget::paintEvent(QPaintEvent* e)
{
	// OCCT handles rendering; Qt's paint engine is disabled in paintEngine()
}

void
OcctWidget::mousePressEvent(QMouseEvent* e)
{
	if (m_controller)
	{
		m_controller->onMousePressEvent(e, devicePixelRatioF());
		update();
	}
}

void
OcctWidget::mouseMoveEvent(QMouseEvent* e)
{
	if (m_controller)
	{
		m_controller->onMouseMoveEvent(e, devicePixelRatioF());
		update();
	}
}

void
OcctWidget::mouseReleaseEvent(QMouseEvent* e)
{
	if (m_controller)
	{
		m_controller->onMouseReleaseEvent(e, devicePixelRatioF());
		update();
	}
}

void
OcctWidget::wheelEvent(QWheelEvent* e)
{
	if (m_controller)
	{
		m_controller->onWheelEvent(e, devicePixelRatioF());
		update();
	}
}

void
OcctWidget::keyPressEvent(QKeyEvent* e)
{
	if (m_controller)
	{
		m_controller->onKeyEvent(e);
		update();
	}
}
