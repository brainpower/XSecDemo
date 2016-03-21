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

#include "xsd_decrypt_dialog.hpp"

#include <QFileInfo>
#include <QMessageBox>

XSDDecryptDialog::XSDDecryptDialog(const QString &file, QWidget *parent) {
	_file = file;
	setupUi();
}


QSize
XSDDecryptDialog::sizeHint() const {
	return QSize(500, 200).expandedTo(minimumSizeHint());
}

void
XSDDecryptDialog::setupUi() {
	setWindowTitle(QStringLiteral("Entschlüsseln"));
	mainLay = new QVBoxLayout(this);
	publicLay = new QFormLayout();

	keyLabel = new QLabel(QStringLiteral("Privater Schlüssel"));
	passwdLine = new QLineEdit();

	keyLine = new FileSelect();
	passwdBox = new QCheckBox(QStringLiteral("Kennwort"));
	saveAsLabel = new QLabel(QStringLiteral("Speichern als:"));
	saveAsLine = new FileSaveSelect();

	buttons = new QDialogButtonBox(this);
	buttons->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Abort);

	passwdLine->setEchoMode( QLineEdit::Password );
	passwdLine->setEnabled( false );

	publicLay->setLabelAlignment(Qt::AlignLeft);
	publicLay->setWidget(0, QFormLayout::LabelRole, keyLabel);
	publicLay->setWidget(0, QFormLayout::FieldRole, keyLine);
	publicLay->setWidget(1, QFormLayout::LabelRole, passwdBox);
	publicLay->setWidget(1, QFormLayout::FieldRole, passwdLine);


	mainLay->addLayout(publicLay);
	mainLay->addWidget(saveAsLabel);
	mainLay->addWidget(saveAsLine);
	mainLay->addWidget(buttons);

	connect(passwdBox, SIGNAL(toggled(bool)), this, SLOT(slotPasswdToggled(bool)));
	connect(buttons, SIGNAL(accepted()), this, SLOT(slotDecrypt()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	if(_file.isEmpty()){
		saveAsLine->setText(QStringLiteral("unbenannt-decrypted.xml"));
	} else {
		QFileInfo fi( _file );
		QString _newfile( "%1/%2-decrypted.xml" );
		saveAsLine->setText( _newfile.arg( fi.path()).arg( fi.baseName()));
	}
}

void XSDDecryptDialog::slotDecrypt() {
	XSec::decrypt_options_t dco;

	auto newfile = saveAsLine->text().toStdString();

	if(passwdBox->isChecked())
		dco.key_password = passwdLine->text().toStdString();
	dco.private_key = keyLine->text().toStdString();
	dco.private_key_is_p12 = (QFileInfo(keyLine->text()).suffix() == QStringLiteral("p12"));

	auto ret = core.decrypt( _file.toStdString(), newfile, dco );
	if( ret != 0 ) {
		QMessageBox box;
		box.setText(QStringLiteral("Ein Fehler ist aufgetreten!"));
		if(!XSec::Core::serror_message().empty()) {
			box.setInformativeText( QStringLiteral("detail: ")+ QString::fromStdString( XSec::Core::serror_message()));
			XSec::Core::serror_reset();
		} else
			box.setInformativeText(QString::fromStdString( core.error_message()));
		box.exec();
	} else {
		QUrl newurl = QUrl::fromLocalFile(QString::fromStdString(newfile));
		emit documentReady(newurl);
		accept();
	}
}
