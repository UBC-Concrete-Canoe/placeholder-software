#pragma once

#include <memory>

class MainWindow;
class ViewportManager;

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
	std::unique_ptr<MainWindow> m_window;
	std::unique_ptr<ViewportManager> m_viewManager;
};
