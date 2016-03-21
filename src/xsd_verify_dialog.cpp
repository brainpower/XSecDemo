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

#include "xsd_verify_dialog.hpp"
#include <QMessageBox>
#include <QFileInfo>

XSDVerifyDialog::XSDVerifyDialog(const QString &file, QWidget *parent) {
	_file = file;
	setupUi();
}

QSize XSDVerifyDialog::sizeHint() const {
	return QSize(400, 200).expandedTo(minimumSizeHint());
}

void XSDVerifyDialog::setupUi() {
	setWindowTitle(QStringLiteral("Verifizieren"));

	mainLay = new QVBoxLayout(this);
	publicLay = new QFormLayout();

	certRadio = new QRadioButton(QStringLiteral("Zertifikat*"));
	keyRadio = new QRadioButton(QStringLiteral("Öffentlicher Schlüssel"));
	noneRadio = new QRadioButton(QStringLiteral("Eingebettet"));
	publicCLine = new FileSelect();
	publicKLine = new FileSelect();
	noteLabel = new QLabel(QStringLiteral("* Achtung: Zertifikatskette wird in diesem Modus nicht geprüft!"));
	trustCert = new QCheckBox(QStringLiteral("Vertraue eingebettetem Zertifikat (für Selbst-Signierte)"));
	//publicNLine = new QLineEdit();

	publicBox = new QGroupBox();
	buttons = new QDialogButtonBox(this);
	buttons->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Abort);

	publicLay->setLabelAlignment(Qt::AlignLeft);
	publicLay->setWidget(0, QFormLayout::LabelRole, certRadio);
	publicLay->setWidget(0, QFormLayout::FieldRole, publicCLine);
	publicLay->setWidget(1, QFormLayout::LabelRole, keyRadio);
	publicLay->setWidget(1, QFormLayout::FieldRole, publicKLine);
	publicLay->setWidget(2, QFormLayout::LabelRole, noneRadio);
	//publicLay->setWidget(2, QFormLayout::FieldRole, publicNLine);

	mainLay->addLayout(publicLay);
	mainLay->addWidget(noteLabel);
	mainLay->addWidget(trustCert);
	mainLay->addWidget(buttons);

	//connect(noneRadio, SIGNAL(toggled(bool)), this, SLOT(slotNoneToggled(bool)));
	connect(certRadio, SIGNAL(toggled(bool)), this, SLOT(slotCertToggled(bool)));
	connect(keyRadio,  SIGNAL(toggled(bool)), this, SLOT(slotKeyToggled(bool)));
	connect(buttons, SIGNAL(accepted()), this, SLOT(slotVerify()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	noneRadio->setChecked(true);
	publicKLine->setEnabled(false);
	publicCLine->setEnabled(false);
}

void XSDVerifyDialog::slotVerify() {
	// verifiziere aktuelle datei
	XSec::verify_options_t so;
	bool res = false;

	if( certRadio->isChecked() ){
		so.public_key_is_cert = true;
		so.public_key = publicCLine->text().toStdString();
		so.trust_selfsigned_cert = trustCert->isChecked();
	} else if(keyRadio->isChecked()) {
		so.public_key_is_cert = false;
		so.public_key = publicKLine->text().toStdString();
	} else {
		so.trust_selfsigned_cert = trustCert->isChecked();
	}

	auto ret = core.verify( _file.toStdString(), res, so );
	if( ret != 0 ) {
		QMessageBox box;
		box.setText(QStringLiteral("Ein Fehler ist aufgetreten!"));
		box.setInformativeText(QString::fromStdString(core.error_message()));
		box.setIcon(QMessageBox::Critical );
		box.exec();
	} else {
		QString message;
		QMessageBox::Icon icon;
		QMessageBox box;
		if( res ){
			message = QStringLiteral("Die Signatur ist gültig.");
			icon = QMessageBox::Information;
		} else {
			message = QStringLiteral("Die Signatur ist ungültig!                   ");
			icon = QMessageBox::Warning;
			if(!XSec::Core::serror_message().empty()) {
				box.setInformativeText( QString::fromStdString( XSec::Core::serror_message()));
				XSec::Core::serror_reset();
			} else
				box.setInformativeText(QString::fromStdString(core.error_message()));
		}
		box.setText(message);
		box.setIcon(icon);

		box.exec();
		accept();
	}
}