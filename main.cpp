
#include <QtWidgets/QApplication>
#include <QtWidgets/QDirModel>
#include <QtWidgets/QFileSystemModel>
#include "Mainwindow.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setStyle(new RibbonStyle);

	app.setApplicationName("QtitanRibbon Controls Sample");
	app.setOrganizationName("Developer Machines");

	MainWindow mainWindow;
	mainWindow.loadRencentProjectConfig();
	//mainWindow.resize(1000, 600);
	mainWindow.showMaximized();

	int nRes = app.exec();
	mainWindow.saveRencentProjectConfig();
	return nRes;
}
