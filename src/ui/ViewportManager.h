#pragma once

#include <memory>
#include "render/OcctViewport.h"

class MainWindow;
class ViewportController;

/**
 * @brief Manages viewport creation and integration with geometry.
 *
 * Manages the four viewports and their respective controllers.
 * Responsible for syncing the viewports to the same geometry and
 * rendering from different perspectives.
 */
class ViewportManager
{
public:
	//! Construct viewports and attach input controllers.
	ViewportManager();
	//! Destructor.
	~ViewportManager();

	/**
	 * @brief Connects the Qt widgets to their respective OCCT viewports.
	 * @param window The main application window.
	 */
	void initializeViewport(MainWindow* window);

	/**
	 * @brief Displays sample geometry for testing.
	 * @param shape The geometry to be rendered in the viewports.
	 */
	void create_shape(TopoDS_Shape shape);

	/**
	 * @brief Orient the planar views with respect to the geometry.
	 */
	void set_planars();

private:
	//! Primary perspective viewport.
	std::unique_ptr<OcctViewport> persp_viewport;
	//! Top/plan orthographic viewport.
	std::unique_ptr<OcctViewport> plan_viewport;
	//! Profile orthographic viewport.
	std::unique_ptr<OcctViewport> profile_viewport;
	//! Body-plan orthographic viewport.
	std::unique_ptr<OcctViewport> bodyplan_viewport;

	//! Input controller for perspective viewport.
	std::unique_ptr<ViewportController> persp_controller;
	//! Input controller for plan viewport.
	std::unique_ptr<ViewportController> plan_controller;
	//! Input controller for profile viewport.
	std::unique_ptr<ViewportController> profile_controller;
	//! Input controller for body-plan viewport.
	std::unique_ptr<ViewportController> bodyplan_controller;
};
