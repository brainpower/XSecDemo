
#ifndef XSD_MAIN_WINDOW_HPP
#define XSD_MAIN_WINDOW_HPP

//core
#include <QString>
#include <QStringList>
#include <QUrl>

//gui
#include <QMainWindow>
#include <QTabWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>

//kde
#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/View>
#include <KTextEditor/ModificationInterface>

#include <KParts/MainWindow>

class KToggleAction;
class KRecentFilesAction;
class KSqueezedTextLabel;

class XSDMainWindow : public KParts::MainWindow {
	Q_OBJECT

	QList<KTextEditor::Document *> docsList;

public:

	XSDMainWindow();
	explicit XSDMainWindow(const QStringList &files);

	virtual ~XSDMainWindow();




private:
	void setupUi();
	void newEmptyTab();
	void newTabWithFile(const QUrl &url);
	void loadURL(const QUrl&);

	bool eventFilter(QObject*, QEvent*);

public slots:
	QSize sizeHint () const;

	void slotOpen();
	void slotNew();
	void slotFlush();
	void slotOpen(const QUrl&);
	void slotTabChanged();
	void slotTabClose(int);
	void aboutEditor();
	void dragEnterEvent(QDragEnterEvent*);
	void dropEvent(QDropEvent*);
	void slotDropEvent(QDropEvent*);

	void slotStartSign();
	void slotStartVerify();
	void slotStartEncrypt();
	void slotStartDecrypt();

	void documentNameChanged(KTextEditor::Document*);

private:
    KRecentFilesAction *recentFiles;
    KToggleAction *m_paShowPath;
    KToggleAction *m_paShowMenuBar;
    KToggleAction *m_paShowStatusBar;

	QTabWidget *centralWidget;
	QAction    *closeAction;
	QAction    *startSignAction, *startVerifyAction, *startEncryptAction, *startDecryptAction;
	KTextEditor::Editor *editor;
	KTextEditor::View   *attachedView;
};

#endif /* ifndef XSD_MAIN_WINDOW_HPP */
