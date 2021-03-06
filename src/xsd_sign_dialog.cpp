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

#include <QMessageBox>
#include <QFileInfo>
#include "xsd_sign_dialog.hpp"


XSDSignDialog::XSDSignDialog(const QString &file, QWidget *parent)
		: QDialog(parent) {
	_file = file;
	setupUi();
}

QSize XSDSignDialog::sizeHint() const {
	return QSize(500, 400).expandedTo(minimumSizeHint());
}

void XSDSignDialog::setupUi() {
	setWindowTitle(QStringLiteral("Signieren"));

	mainLay = new QVBoxLayout(this);
	refsLay = new QHBoxLayout();
	buttonLay = new QVBoxLayout();
	upperLay = new QFormLayout();
	publicLay = new QFormLayout();

	addBtn = new QPushButton(QStringLiteral("Hinzufügen"), this);
	removeBtn = new QPushButton(QStringLiteral("Reset"), this);
	editBtn = new QPushButton(QStringLiteral("Bearbeiten"), this);
	formLabel = new QLabel(QStringLiteral("Form"));
	signLabel = new QLabel(QStringLiteral("Signiermethode"));
	canonLabel = new QLabel(QStringLiteral("Kanonisierung"));
	refsLabel = new QLabel(QStringLiteral("Referenzen"));
	privateLabel = new QLabel(QStringLiteral("Privater Schlüssel"));
	saveAsLabel = new QLabel(QStringLiteral("Speichern als:"));
	passwdBox = new QCheckBox(QStringLiteral("Kennwort"));
	certRadio = new QRadioButton(QStringLiteral("Zertifikat"));
	keyRadio = new QRadioButton(QStringLiteral("Öffentlicher Schlüssel"));
	p12Radio = new QRadioButton(QStringLiteral("Zertifikat in P12 Datei"));
	publicCLine = new FileSelect();
	publicKLine = new FileSelect();
	passwdLine = new QLineEdit();
	privateLine = new FileSelect();
	saveAsLine = new FileSaveSelect();
	formBox = new QComboBox();
	signBox = new QComboBox();
	canonBox = new QComboBox();
	refsList = new QListWidget();
	publicBox = new QGroupBox();
	buttons = new QDialogButtonBox(this);
	buttons->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Abort);

	passwdLine->setEchoMode( QLineEdit::Password );
	passwdLine->setEnabled( false );

	formBox->addItem(QStringLiteral("Enveloped"), XSec::SF_ENVELOPED );
	formBox->addItem(QStringLiteral("Enveloping"), XSec::SF_ENVELOPING );
	formBox->addItem(QStringLiteral("Detached"),  XSec::SF_DETACHED );

	signBox->addItem(QStringLiteral("RSA-SHA1"),   XSec::SA_RSA_SHA1 );
	signBox->addItem(QStringLiteral("RSA-SHA224"), XSec::SA_RSA_SHA224 );
	signBox->addItem(QStringLiteral("RSA-SHA256"), XSec::SA_RSA_SHA256 );
	signBox->addItem(QStringLiteral("RSA-SHA384"), XSec::SA_RSA_SHA384 );
	signBox->addItem(QStringLiteral("RSA-SHA512"), XSec::SA_RSA_SHA512 );
	signBox->addItem(QStringLiteral("ECDSA-SHA1"),   XSec::SA_ECDSA_SHA1 );
	signBox->addItem(QStringLiteral("ECDSA-SHA224"), XSec::SA_ECDSA_SHA224 );
	signBox->addItem(QStringLiteral("ECDSA-SHA256"), XSec::SA_ECDSA_SHA256 );
	signBox->addItem(QStringLiteral("ECDSA-SHA384"), XSec::SA_ECDSA_SHA384 );
	signBox->addItem(QStringLiteral("ECDSA-SHA512"), XSec::SA_ECDSA_SHA512 );

	canonBox->addItem(QStringLiteral("Kanonisierung inklusiv 1.1"), XSec::C14N_11_INCLUSIVE);
	canonBox->addItem(QStringLiteral("Kanonisierung inklusiv"), XSec::C14N_INCLUSIVE);
	canonBox->addItem(QStringLiteral("Kanonisierung exklusiv"), XSec::C14N_EXCLUSIVE);

	buttonLay->addWidget(addBtn);
	buttonLay->addWidget(editBtn);
	buttonLay->addWidget(removeBtn);

	refsLay->addWidget(refsList);
	refsLay->addLayout(buttonLay);

	upperLay->setFieldGrowthPolicy( QFormLayout::AllNonFixedFieldsGrow );
	upperLay->setLabelAlignment(Qt::AlignLeft);
	upperLay->setWidget(0, QFormLayout::LabelRole, formLabel);
	upperLay->setWidget(0, QFormLayout::FieldRole, formBox);
	upperLay->setWidget(1, QFormLayout::LabelRole, signLabel);
	upperLay->setWidget(1, QFormLayout::FieldRole, signBox);
	upperLay->setWidget(2, QFormLayout::LabelRole, canonLabel);
	upperLay->setWidget(2, QFormLayout::FieldRole, canonBox);
	upperLay->setWidget(3, QFormLayout::LabelRole, privateLabel);
	upperLay->setWidget(3, QFormLayout::FieldRole, privateLine);
	upperLay->setWidget(4, QFormLayout::LabelRole, passwdBox);
	upperLay->setWidget(4, QFormLayout::FieldRole, passwdLine);

	publicLay->setLabelAlignment(Qt::AlignLeft);
	publicLay->setWidget(0, QFormLayout::LabelRole, certRadio);
	publicLay->setWidget(0, QFormLayout::FieldRole, publicCLine);
	publicLay->setWidget(1, QFormLayout::LabelRole, keyRadio);
	publicLay->setWidget(1, QFormLayout::FieldRole, publicKLine);
	publicLay->setWidget(2, QFormLayout::LabelRole, p12Radio);

	mainLay->addLayout(upperLay);
	mainLay->addLayout(publicLay);
	mainLay->addWidget(refsLabel);
	mainLay->addLayout(refsLay);
	mainLay->addWidget(saveAsLabel);
	mainLay->addWidget(saveAsLine);
	mainLay->addWidget(buttons);


	connect(passwdBox, SIGNAL(toggled(bool)), this, SLOT(slotPasswdToggled(bool)));
	connect(certRadio, SIGNAL(toggled(bool)), this, SLOT(slotCertToggled(bool)));
	connect(keyRadio,  SIGNAL(toggled(bool)), this, SLOT(slotKeyToggled(bool)));
	connect(buttons, SIGNAL(accepted()), this, SLOT(slotSign()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	connect(addBtn, SIGNAL(clicked()), this, SLOT(slotAddRef()));
	connect(removeBtn, SIGNAL(clicked()), this, SLOT(slotDelRef()));
	connect(editBtn, SIGNAL(clicked()), this, SLOT(slotEditRef()));

	certRadio->setChecked(true);
	publicKLine->setEnabled(false);

	if(_file.isEmpty()){
		saveAsLine->setText(QStringLiteral("unbenannt-signed.xml"));
	} else {
		QFileInfo fi( _file );
		QString _newfile( "%1/%2-signed.xml" );
		saveAsLine->setText( _newfile.arg( fi.path()).arg( fi.baseName()));
	}

}

void XSDSignDialog::slotAddRef() {
		XSDReferenceDialog r(0);
		if(r.exec()){
			auto ref = new XSec::Reference(r.ref());
			_refs.push_back(ref);
			auto it = new QListWidgetItem(QStringLiteral("URI = \"") + QString::fromStdString(ref->uri) + QStringLiteral("\""));
			it->setData(Qt::UserRole, (qulonglong) _refs.size()-1 );
			refsList->addItem(it);
		}
}

void XSDSignDialog::slotEditRef() {
	for( auto item : refsList->selectedItems() ){
		auto ref = _refs[ item->data(Qt::UserRole).toULongLong() ];
		XSDReferenceDialog r(*ref);
		if(r.exec()) {
			*ref = r.ref();
			item->setText(QStringLiteral("URI = \"") + QString::fromStdString(ref->uri) + QStringLiteral("\""));
		}
	}
}

void XSDSignDialog::slotDelRef() {
	refsList->selectAll();
	for( auto item : refsList->selectedItems() ){
		auto ref = _refs [ item->data(Qt::UserRole).toULongLong() ];
		delete item;
		delete ref;
	}
	_refs.erase(_refs.begin(), _refs.end());
}

void XSDSignDialog::slotSign() {
	//siniere aktuelle datei
	XSec::sign_options_t so;

	auto newfile = saveAsLine->text().toStdString();

	so.format = formBox->currentData().toInt();

	if(so.format == XSec::SF_ENVELOPED ){
		if( !QFile(_file).exists() && _refs.empty() ){
			QMessageBox box;
			box.setText(QStringLiteral("Ein Fehler ist aufgetreten!"));
			QString info("Datei '%1' konnte nicht geöffnet werden.");
			box.setInformativeText(info.arg(_file));
			box.exec();
			return;
		}
	}

	so.sign_algorithm = signBox->currentData().toInt();
	so.c14n_algorithm = canonBox->currentData().toInt();
	so.private_key = privateLine->text().toStdString();

	if( QFileInfo(privateLine->text()).suffix() == QStringLiteral("p12")){
		so.keys_in_p12 = true;
	}

	if( passwdBox->isChecked() )
		so.key_password = passwdLine->text().toStdString();

	if( certRadio->isChecked() ){
		so.public_key_is_cert = true;
		so.public_key = publicCLine->text().toStdString();
	} else if (keyRadio->isChecked()) {
		so.public_key_is_cert = false;
		so.public_key = publicKLine->text().toStdString();
	} else { // P12 Datei!
		so.keys_in_p12 = true;
	}

	if( so.format != XSec::SF_ENVELOPED && _refs.empty()){
		auto r = new XSec::Reference;
		r->uri = QUrl::fromLocalFile(_file).toString().toStdString();
		r->hash = XSec::HA_UNSET;
		r->transform = XSec::C14N_UNSET;
		_refs.push_back(r);
	}
	so.references = _refs;

	auto ret = core.sign( _file.toStdString(), newfile, so );
	if( ret != 0 ) {
		QMessageBox box;
		box.setText(QStringLiteral("Ein Fehler ist aufgetreten!"));
		box.setInformativeText(QString::fromStdString(core.error_message()));
		box.exec();
	} else {
		QUrl newurl = QUrl::fromLocalFile(QString::fromStdString(newfile));
		emit documentReady(newurl);
		accept();
	}
}
