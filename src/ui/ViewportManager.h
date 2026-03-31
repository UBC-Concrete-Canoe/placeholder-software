#pragma once

#include <memory>
#include "core/HullModel.h"
#include "render/OcctViewport.h"
#include "render/WireframeManager.h"

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
	 * @brief Orient the planar views with respect to the geometry.
	 */
	void set_planars();

	/**
	 * @brief Build and display a demo HullModel with visual points and wireframe.
	 */
	void create_demo_hull_model();

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

	//! Backing model for demo content so displayed control-point pointers remain valid.
	std::shared_ptr<HullModel> demo_hull_model;
	//! Wireframe renderer bound to the demo model.
	std::unique_ptr<WireframeManager> demo_wireframe_manager;
};
