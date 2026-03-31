#pragma once
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include "render/OcctViewport.h"
#include "app/MoveTool.h"
#include "core/HullModel.h"
#include <memory>

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
     * @brief Attach a hull model to enable point picking and dragging.
     *
     * Creates the internal MoveTool using the viewport's current AIS context
     * and view. Must be called after initialize() so the context and view
     * are valid.
     *
     * @param model Hull model whose control points will be selectable and draggable.
     */
    void setModel(HullModel* model);

private:
	void synchronizeAndFlush();
	bool m_leftButtonPressed = false;
	bool m_leftButtonDragged = false;
	Graphic3d_Vec2i m_leftPressPos = Graphic3d_Vec2i(0, 0);
	OcctViewport* m_viewport;
    std::unique_ptr<MoveTool> m_moveTool;
};
