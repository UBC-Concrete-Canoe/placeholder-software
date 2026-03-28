#include "Application.h"
#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include "app/ViewportController.h"
#include "render/OcctViewport.h"
#include "ui/MainWindow.h"
#include "ui/OcctWidget.h"

//for testing
#include "core/HullModel.h"
#include "render/WireframeManager.h"
#include <memory>

Application::Application() = default;

void
Application::run()
{
	MainWindow* window = new MainWindow();
	OcctWidget* occt_widget = new OcctWidget();
	OcctViewport* occt_viewport = new OcctViewport();

	// Create the controller to handle user input
	ViewportController* viewport_controller = new ViewportController(occt_viewport);

	// Set the OCCT widget as the main window's central widget
	window->setCentralWidget(occt_widget);

	// Show the window to create the native window handle
	window->show();

	// Ensure Qt creates a native handle; required for OCCT embedding
	(void)occt_widget->winId();
	occt_widget->setController(viewport_controller);

	// Initialize OCCT within the widget's native window
	initializeGraphics(occt_widget, occt_viewport);

	// --- NEW WIREFRAME TEST CODE ---

	// 1. Create a 4x4 test grid
	std::shared_ptr<HullModel> testModel = std::make_shared<HullModel>(4, 4);

	// 2. Spread the points out into a visible 3D grid
	int currentId = 0;
	for (int u = 0; u < 4; ++u) {
		for (int v = 0; v < 4; ++v) {
			// Create a gentle curve by raising the Z height of the middle points
			double zHeight = (u > 0 && u < 3 && v > 0 && v < 3) ? 10.0 : 0.0;
			testModel->updatePoint(currentId, gp_Pnt(u * 30.00, v *30.0, zHeight));
			currentId++;
		}
	}

	// 3. Create the manager to draw the grid
	WireframeManager* testManager = new WireframeManager(occt_viewport->getContext(), testModel);

	// 4. Build the initial lattice
	testManager->BuildLattice();

	// 5. Tell the viewport to zoom to our new wireframe
	occt_viewport->fitAll();
	
}

void
Application::initializeGraphics(OcctWidget* widget, OcctViewport* viewport)
{
	// Pass the widget to the viewport
	viewport->initialize(widget->winId());
}

Application::~Application(void) {}
