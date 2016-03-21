
//core
#include <QApplication>
#include <QCommandLineParser>

//gui
#include <QMainWindow>

//kde
#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KAboutData>

// own
#include "xsdmainwindow.hpp"

#ifndef i18n
#define i18n QStringLiteral
#endif

int main(int argc, char **argv){

	QApplication app(argc, argv);

	// windws needs this somehow
	QIcon::setThemeName(QStringLiteral("oxygen"));

	KAboutData aboutData(QStringLiteral("xsecdemo"),
                         i18n("XSecDemo"),
                         QStringLiteral("0.1.0"),
                         i18n("XSecDemo"), KAboutLicense::Custom,
                         i18n("(c) 2016 Franz Baumgärtner"), QString(), QStringLiteral("http://haw-landshut.de"));
	aboutData.setBugAddress("");
	aboutData.setLicenseText("Copyright (c) 2016 brainpower <fbaumgae at haw-landshut dot de>\n"
			                         "\n"
			                         "Permission is hereby granted, free of charge, to any person obtaining a copy\n"
			                         "of this software and associated documentation files (the \"Software\"), to deal\n"
			                         "in the Software without restriction, including without limitation the rights\n"
			                         "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
			                         "copies of the Software, and to permit persons to whom the Software is\n"
			                         "furnished to do so, subject to the following conditions:\n"
			                         "\n"
			                         "The above copyright notice and this permission notice shall be included in\n"
			                         "all copies or substantial portions of the Software.\n"
			                         "\n"
			                         "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
			                         "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
			                         "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
			                         "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
			                         "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
			                         "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN\n"
			                         "THE SOFTWARE." );
	KAboutData::setApplicationData(aboutData);

	// hiDPI, if neccessary
	//app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

	app.setApplicationName("xsecdemo");
	app.setApplicationDisplayName("XML Security Demo");
	app.setOrganizationName("Baumgaertner");
	app.setOrganizationDomain("de.baumgaertner");
	app.setApplicationVersion("0.1.0");

	// programm's icon
	QApplication::setWindowIcon(QIcon::fromTheme(QLatin1String("accessories-text-editor")));

	QCommandLineParser parser;
	parser.setApplicationDescription("Demonstrator for XML Security");
	parser.addVersionOption();
	parser.addHelpOption();

	parser.addPositionalArgument(QStringLiteral("files"), QStringLiteral("Files to open."), QStringLiteral("[files...]"));

	parser.process(app);

	XSDMainWindow *xmw;
	if (parser.positionalArguments().count() == 0) {
		xmw = new XSDMainWindow;
	} else {
		xmw = new XSDMainWindow(parser.positionalArguments());
	}

	xmw->show();
	return app.exec();
}
