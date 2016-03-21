/*
 * Copyright (c) 2015-2016 brainpower <fbaumgae at haw-landshut dot de>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*/


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
