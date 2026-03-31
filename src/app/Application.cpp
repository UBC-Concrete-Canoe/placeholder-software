#include "Application.h"
#include "ui/MainWindow.h"
#include "ui/ViewportManager.h"
// OCCT helper for creating a test box
#include <BRepPrimAPI_MakeBox.hxx>

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

	// Create and display a demo object
	TopoDS_Shape box = BRepPrimAPI_MakeBox(10.0, 10.0, 20.0).Shape();
	m_viewManager->create_shape(box);
	m_viewManager->create_demo_control_points();

	// Configure planar views
	m_viewManager->set_planars();
}

Application::~Application(void) {}
