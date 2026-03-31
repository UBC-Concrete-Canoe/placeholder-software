#include <QApplication>
#include "app/Application.h"

// Linux specific
#ifdef Q_OS_LINUX
#include <cstdlib>
#endif

/**
 * @brief Program entry point.
 *
 * Initializes Qt and starts the application event loop.
 * On Linux, forces X11/XWayland because OCCT rendering is not supported on Wayland.
 */
int
main(int argc, char* argv[])
{

	// OpenCascade does not work under wayland, need to force X11/XWayland
#ifdef Q_OS_LINUX
	setenv("QT_QPA_PLATFORM", "xcb", 1);
#endif

	QApplication app(argc, argv);
	Application application;
	application.run();
	return app.exec();
}
