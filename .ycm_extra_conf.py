
flags = [
	"-x", "c++",
	"-std=gnu++14",
	"-I../src",
	"-I.",
	"-isystem", "/usr/include/qt",
	"-isystem", "/usr/include/qt/QtWidgets",
	"-isystem", "/usr/include/qt/QtGui",
	"-isystem", "/usr/include/qt/QtSql",
	"-isystem", "/usr/include/qt/QtCore",
	"-isystem", "/usr/include/KF5/KTextEditor",
	"-I/usr/lib/qt/mkspecs/linux-g++",
	"-DBP_QT5",
	"-DQT_WIDGETS_LIB",
	"-DQT_GUI_LIB",
	"-DQT_SQL_LIB",
	"-DQT_CORE_LIB",
	"-fPIC"
]

def FlagsForFile( filename, **kwargs ):
	return { 'flags': flags, 'do_cache': True }
