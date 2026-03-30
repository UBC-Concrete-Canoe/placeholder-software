#pragma once
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include "render/OcctViewport.h"

// Inherit from AIS_ViewController to get standard OCCT mouse math (orbit/zoom/pan calculations)
#include <AIS_ViewController.hxx>

/**
 * @brief Controller layer for viewport interaction.
 *
 * Interprets Qt user input (mouse, keyboard) and translates it into OpenCascade view
 * transformations (orbit, pan, zoom). Inherits from AIS_ViewController to leverage
 * OCCT's built-in interaction mathematics.
 */
class ViewportController : public AIS_ViewController
{
public:
	/**
	 * @brief Constructor.
	 * @param viewport The viewport this controller will manipulate (ownership with caller)
	 */
	ViewportController(OcctViewport* viewport);

	/**
	 * @brief Handle mouse button press.
	 * Initiates interaction (rotation, zoom, or pan) based on button.
	 */
	void onMousePressEvent(QMouseEvent* e);

	/**
	 * @brief Handle mouse button release.
	 * Ends the current interaction.
	 */
	void onMouseReleaseEvent(QMouseEvent* e);

	/**
	 * @brief Handle mouse movement.
	 * Updates view transformation while button is held.
	 */
	void onMouseMoveEvent(QMouseEvent* e);

	/**
	 * @brief Handle mouse wheel scroll.
	 * Zooms the view in/out.
	 */
	void onWheelEvent(QWheelEvent* e);

	/**
	 * @brief Handle keyboard input.
	 * Performs view presets and display mode toggles (F=fit, S=shaded, W=wireframe, etc).
	 */
	void onKeyEvent(QKeyEvent* e);

	/**
	 * @brief Handle window resize.
	 * Notifies the view of geometry changes.
	 */
	void onResize();

	/**
	 * @brief Enable or disable mouse-driven orbit rotation.
	 * @param enabled False to block left-button orbit while keeping pan/zoom active.
	 */
	void setRotationEnabled(bool enabled);

private:
	OcctViewport* m_viewport;
	bool m_rotationEnabled = true;
};