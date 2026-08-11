#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
} // namespace Ui
QT_END_NAMESPACE

/**
 * @brief Top-level Qt window that hosts the viewport layout and controls.
 */
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	/**
	 * @brief Construct the main application window.
	 * @param parent Optional parent widget.
	 */
	explicit MainWindow(QWidget* parent = nullptr);
	//! Destroy owned UI resources.
	~MainWindow();

private:
	//! Designer-generated UI object.
	Ui::MainWindow* ui;
};
#endif // MAINWINDOW_H
