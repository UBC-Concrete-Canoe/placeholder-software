#pragma once
#include "core/HullModel.h"
#include "render/OcctViewport.h"
#include "ui/OcctWidget.h"

/**
 * @brief Application entry point and initialization.
 *
 * Handles startup logic including window creation, OCCT viewport
 * initialization, and scene population.
 */
class Application
{
public:
	//! Constructor.
	Application();
	//! Destructor.
	~Application();

	/**
	 * @brief Initialize and run the application.
	 *
	 * Creates the main window, viewport, and controller, then displays
	 * the initial scene.
	 */
	void run();

private:
	/**
	 * @brief Initialize OCCT rendering in the given widget.
	 * @param widget The OcctWidget receiving the viewport
	 * @param viewport The OcctViewport to initialize
	 */
	void initializeGraphics(OcctWidget* widget, OcctViewport* viewport);

	HullModel m_hullModel;
};
