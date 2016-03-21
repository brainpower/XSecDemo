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


#include "xsdmainwindow.hpp"
#include "xsd_reference_dialog.hpp"
#include "xsd_sign_dialog.hpp"
#include "xsd_verify_dialog.hpp"
#include "xsd_decrypt_dialog.hpp"
#include "xsd_encrypt_dialog.hpp"

#include <KStandardAction>
#include <KActionCollection>
#include <KToggleAction>
#include <KRecentFilesAction>
#include <KToggleAction>
#include <KXMLGUIFactory>
#include <KAboutApplicationDialog>

#include <QApplication>
#include <QFileOpenEvent>
#include <QDir>
#include <QRegularExpression>
#include <QFileDialog>
#include <QMimeData>
#include <QMessageBox>

#ifndef i18n
#define i18n QStringLiteral
#endif


XSDMainWindow::XSDMainWindow(){
	setupUi();

	editor = KTextEditor::Editor::instance();

	newEmptyTab();
}

XSDMainWindow::XSDMainWindow(const QStringList &files ){
	setupUi();
	editor = KTextEditor::Editor::instance();

	QUrl url;
	for( auto &file : files ){
		const QRegularExpression withProtocol(QStringLiteral("^[a-zA-Z]+://")); // TODO: remove after Qt supports this on its own

    if (withProtocol.match(file).hasMatch()) {
      url = QUrl::fromUserInput(file);
    } else {
      url = QUrl::fromLocalFile(QDir::current().absoluteFilePath(file));
    }

    if (url.isLocalFile() && QFile::exists(file)) {
			slotOpen(url);
		}
	}
}

XSDMainWindow::~XSDMainWindow(){
	// disconnect here, or we get problems in slotTabChanged because deleting docs deletes their views
	centralWidget->disconnect(SIGNAL(currentChanged(int)));

	for( auto doc : docsList){
		delete doc;
	}
}

void
XSDMainWindow::setupUi(){
	attachedView = nullptr;
	centralWidget = new QTabWidget(this);
	connect(centralWidget, SIGNAL(currentChanged(int)), this, SLOT(slotTabChanged()));

	centralWidget->setTabsClosable(true);
	connect(centralWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(slotTabClose(int)));

	startSignAction = actionCollection()->addAction(QStringLiteral("start_sign_action"), this, SLOT(slotStartSign()) );
	startSignAction->setWhatsThis( QStringLiteral( "Start signing the current document." ) );
	startSignAction->setText( QStringLiteral("Signieren"));
	startSignAction->setIcon( QIcon::fromTheme(QStringLiteral("document-edit-sign")));

	startVerifyAction = actionCollection()->addAction(QStringLiteral("start_verify_action"), this, SLOT(slotStartVerify()) );
	startVerifyAction->setWhatsThis( QStringLiteral( "Start verifying the current document." ));
	startVerifyAction->setText( QStringLiteral("Verifizieren"));
	startVerifyAction->setIcon( QIcon::fromTheme(QStringLiteral("document-edit-verify")));

	startEncryptAction = actionCollection()->addAction(QStringLiteral("start_encrypt_action"), this, SLOT(slotStartEncrypt()) );
	startEncryptAction->setWhatsThis( QStringLiteral( "Start encrypting the current document." ));
	startEncryptAction->setText( QStringLiteral("Verschlüsseln"));
	startEncryptAction->setIcon( QIcon::fromTheme(QStringLiteral("document-edit-encrypt")));

	startDecryptAction = actionCollection()->addAction(QStringLiteral("start_decrypt_action"), this, SLOT(slotStartDecrypt()) );
	startDecryptAction->setWhatsThis( QStringLiteral( "Start decrypting the current document." ));
	startDecryptAction->setText( QStringLiteral("Entschlüsseln"));
	startDecryptAction->setIcon( QIcon::fromTheme(QStringLiteral("document-edit-decrypt")));

  closeAction = actionCollection()->addAction(KStandardAction::Close, QStringLiteral("file_close"), this, SLOT(slotFlush()));
  closeAction->setWhatsThis(QStringLiteral("Use this command to close the current document"));
  closeAction->setEnabled(false);
  actionCollection()->addAction(KStandardAction::Quit, this, SLOT(close()))
    ->setWhatsThis(QStringLiteral("Close the current document view"));

  // setup File menu
  actionCollection()->addAction(KStandardAction::New, QStringLiteral("file_new"), this, SLOT(slotNew()))
		->setWhatsThis(QStringLiteral("Use this command to create a new document"));
  actionCollection()->addAction(KStandardAction::Open, QStringLiteral("file_open"), this, SLOT(slotOpen()))
    ->setWhatsThis(QStringLiteral("Use this command to open an existing document for editing"));

	recentFiles = KStandardAction::openRecent(this, SLOT(slotOpen(QUrl)), this);
  actionCollection()->addAction(recentFiles->objectName(), recentFiles);
  recentFiles->setWhatsThis(i18n("This lists files which you have opened recently, and allows you to easily open them again."));
	QAction *a;

    a = actionCollection()->addAction(QStringLiteral("help_about_editor"));
    a->setText(i18n("&About Editor Component"));
    connect(a, SIGNAL(triggered()), this, SLOT(aboutEditor()));

  setStandardToolBarMenuEnabled(true);

	setCentralWidget(centralWidget);

  setXMLFile(QStringLiteral("xsecdemoui.rc"));
  createShellGUI(true);

  qApp->installEventFilter(this);
}


QSize
XSDMainWindow::sizeHint() const {
	// set a sensible default window size hint
	return (QSize(1024, 768).expandedTo(minimumSizeHint()));
}

void
XSDMainWindow::slotFlush(){
	slotTabClose(centralWidget->currentIndex());
}

void
XSDMainWindow::documentNameChanged(KTextEditor::Document *doc){
	auto views = doc->views();
	if( !views.isEmpty() ){
		auto index = centralWidget->indexOf(views[0]);
		centralWidget->setTabText(index, doc->documentName());
	}
}

void
XSDMainWindow::slotTabClose(int index){
	auto view = qobject_cast<KTextEditor::View *>(centralWidget->widget(index));
	if (view->document()->closeUrl()) {
		centralWidget->removeTab(index);
		auto doc = view->document();
		delete view;
		if( view == attachedView )
			attachedView = nullptr;

		// if all views are deleted/closed, delete document
		if(doc->views().isEmpty()){
			docsList.removeAll(doc);
			delete doc;
		}

		if(centralWidget->count() == 0){
			closeAction->setDisabled(true);
		}
	}

}

void
XSDMainWindow::slotOpen(){
	auto view = qobject_cast<KTextEditor::View *>(centralWidget->currentWidget());
	QList<QUrl> files;

	if(view){
		files = QFileDialog::getOpenFileUrls(this,
	                        QStringLiteral("Open File"), view->document()->url());
	} else {
		files = QFileDialog::getOpenFileUrls(this, QStringLiteral("Open File"));
	}
  for(QUrl file : files) {
		slotOpen(file);
	}
}

void
XSDMainWindow::slotOpen(const QUrl &url){
	if (url.isEmpty()) return;

	auto view = qobject_cast<KTextEditor::View *>(centralWidget->currentWidget());

	// if there is no current tab, if the document in the current tab is modified
	// or if the current document is an opened file, open new files in new tabs
	// if not, open file in current tab
	if (!view || view->document()->isModified() || !view->document()->url().isEmpty()) {
		newTabWithFile(url);
	} else {
		view->document()->openUrl(url);
	}

	// activate close buttons
	closeAction->setEnabled(true);
}

void
XSDMainWindow::slotNew(){
	newEmptyTab();
}

void
XSDMainWindow::slotTabChanged(){
	auto view = qobject_cast<KTextEditor::View *>(centralWidget->currentWidget());

	if(attachedView){
		guiFactory()->removeClient(attachedView);
		attachedView = nullptr;
	}
	if(view){
		guiFactory()->addClient(view);
		attachedView = view;
	}
}

void
XSDMainWindow::newTabWithFile(const QUrl &url){

		KTextEditor::Document *doc = editor->createDocument(this);

		// enable the modified on disk warning dialogs if any
		if (qobject_cast<KTextEditor::ModificationInterface *>(doc)) {
			qobject_cast<KTextEditor::ModificationInterface *>(doc)->setModifiedOnDiskWarning(true);
		}

		auto view = doc->createView(this);

    view->document()->openUrl(url);

		centralWidget->addTab(view, doc->documentName());
		centralWidget->setCurrentWidget(view);

		connect(doc, SIGNAL(documentNameChanged(KTextEditor::Document*)), this, SLOT(documentNameChanged(KTextEditor::Document*)));

		docsList.append(doc);
}
void
XSDMainWindow::newEmptyTab(){

		KTextEditor::Document *doc = editor->createDocument(this);

		// enable the modified on disk warning dialogs if any
		if (qobject_cast<KTextEditor::ModificationInterface *>(doc)) {
			qobject_cast<KTextEditor::ModificationInterface *>(doc)->setModifiedOnDiskWarning(true);
		}

		auto view = doc->createView(this);

		centralWidget->addTab(view, doc->documentName());
		centralWidget->setCurrentWidget(view);

		connect(doc, SIGNAL(documentNameChanged(KTextEditor::Document*)), this, SLOT(documentNameChanged(KTextEditor::Document*)));

		docsList.append(doc);
}
bool
XSDMainWindow::eventFilter(QObject *obj, QEvent *event){
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *foe = static_cast<QFileOpenEvent*>(event);
        slotOpen(foe->url());
        return true;
    }
    return KParts::MainWindow::eventFilter(obj, event);
}
void
XSDMainWindow::aboutEditor(){
    KAboutApplicationDialog abt(editor->aboutData(), this);
    abt.exec();
}
void
XSDMainWindow::dragEnterEvent(QDragEnterEvent *event){
	qDebug("drag enter");
    const QList<QUrl> uriList = event->mimeData()->urls();
    event->setAccepted(!uriList.isEmpty());
}

void
XSDMainWindow::dropEvent(QDropEvent *event){
    slotDropEvent(event);
}

void
XSDMainWindow::slotDropEvent(QDropEvent *event){
	qDebug("drop");
	const QList<QUrl> textlist = event->mimeData()->urls();

	for(const QUrl & url : textlist){
		slotOpen(url);
	}
}

void XSDMainWindow::slotStartVerify() {

	auto url = attachedView->document()->url();

	if( url.isLocalFile() && QFile::exists( url.toLocalFile() )){
		if( !XSec::Core::hasSignature( url.toLocalFile().toStdString() ) ){
			QMessageBox box;
			box.setText("Warnung: Dokument scheint keine Signatur zu enthalten!");
			box.setInformativeText( "Trotzdem fortfaren?" );
			box.setStandardButtons( QMessageBox::Ok | QMessageBox::Cancel );
			auto ret = box.exec();
			if( ret != QMessageBox::Ok ){
				return;
			}
		}
		XSDVerifyDialog diag(url.toLocalFile());
		diag.exec();
	} else {
		QMessageBox box;
		box.setText(QStringLiteral("Ein Fehler ist aufgetreten!"));
		QString info("Datei '%1' konnte nicht geöffnet werden.");
		box.setInformativeText(info.arg(url.toLocalFile()));
		box.exec();
	}
}

void XSDMainWindow::slotStartSign() {
	XSDSignDialog *test = nullptr;
	auto url = attachedView->document()->url();

	test = new XSDSignDialog(url.toLocalFile());
	connect(test, SIGNAL(documentReady(const QUrl&)), this, SLOT(slotOpen(const QUrl&)));
	test->exec();

	test->deleteLater();
}

void XSDMainWindow::slotStartEncrypt() {
	auto url = attachedView->document()->url();

	if( url.isLocalFile() && QFile::exists( url.toLocalFile() )){
		if( XSec::Core::isEncrypted( url.toLocalFile().toStdString() ) ){
			QMessageBox box;
			box.setText("Warnung: Dokument scheint bereits eine Verschlüsselung zu enthalten!");
			box.setInformativeText( "Trotzdem fortfaren?" );
			box.setStandardButtons( QMessageBox::Ok | QMessageBox::Cancel );
			auto ret = box.exec();
			if( ret != QMessageBox::Ok ){
				return;
			}
		}
		auto diag = new XSDEncryptDialog(url.toLocalFile());
		connect(diag, SIGNAL(documentReady(const QUrl&)), this, SLOT(slotOpen(const QUrl&)));
		diag->exec();
		delete diag;
	} else {
		QMessageBox box;
		box.setText(QStringLiteral("Ein Fehler ist aufgetreten!"));
		QString info("Datei '%1' konnte nicht geöffnet werden.");
		box.setInformativeText(info.arg(url.toLocalFile()));
		box.exec();
	}
}

void XSDMainWindow::slotStartDecrypt() {
	auto url = attachedView->document()->url();

	if( url.isLocalFile() && QFile::exists( url.toLocalFile() )){
		if( !XSec::Core::isEncrypted( url.toLocalFile().toStdString() ) ){
			QMessageBox box;
			box.setText("Warnung: Dokument scheint keine Verschlüsselung zu enthalten!");
			box.setInformativeText( "Trotzdem fortfaren?" );
			box.setStandardButtons( QMessageBox::Ok | QMessageBox::Cancel );
			auto ret = box.exec();
			if( ret != QMessageBox::Ok ){
				return;
			}
		}
		auto diag = new XSDDecryptDialog(url.toLocalFile());
		connect(diag, SIGNAL(documentReady(const QUrl&)), this, SLOT(slotOpen(const QUrl&)));
		diag->exec();
		delete diag;
	} else {
		QMessageBox box;
		box.setText(QStringLiteral("Ein Fehler ist aufgetreten!"));
		QString info("Datei '%1' konnte nicht geöffnet werden.");
		box.setInformativeText(info.arg(url.toLocalFile()));
		box.exec();
	}
}
