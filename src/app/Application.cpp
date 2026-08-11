#include "Application.h"
#include "ui/MainWindow.h"
#include "ui/ViewportManager.h"

Application::Application() = default;

void
Application::run()
{
	m_window = std::make_unique<MainWindow>();
	// Show the window to create the native window handle
	m_window->show();

	// Create helper
	m_viewManager = std::make_unique<ViewportManager>();
	m_viewManager->initializeViewport(m_window.get());
	m_viewManager->create_demo_hull_model();

	// Configure planar views
	m_viewManager->set_planars();
}

Application::~Application(void) {}
