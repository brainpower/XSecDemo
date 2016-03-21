#include "xsd_encrypt_dialog.hpp"

#include <QFileInfo>
#include <QMessageBox>

XSDEncryptDialog::XSDEncryptDialog(const QString &file, QWidget *parent) {
	_file = file;
	setupUi();
}

QSize
XSDEncryptDialog::sizeHint() const {
	return QSize(500, 400).expandedTo(minimumSizeHint());
}

void
XSDEncryptDialog::setupUi() {
	setWindowTitle(QStringLiteral("Verschlüsseln"));

	mainLay = new QVBoxLayout(this);
	upperLay = new QFormLayout();
	publicLay = new QFormLayout();

	typeLabel = new QLabel(QStringLiteral("Algorithmus"));
	formLabel = new QLabel(QStringLiteral("Form"));
	saveAsLabel = new QLabel(QStringLiteral("Speichern als:"));
	xpathLabel = new QLabel(QStringLiteral("Elemente die Verschlüsselt werden als XPath (einer pro Zeile):"));
	passwdBox = new QCheckBox(QStringLiteral("Kennwort"));
	certRadio = new QRadioButton(QStringLiteral("Zertifikat"));
	keyRadio = new QRadioButton(QStringLiteral("Öffentlicher Schlüssel"));
	p12Radio = new QRadioButton(QStringLiteral("Zertifikat in P12 Datei"));
	publicCLine = new FileSelect();
	publicKLine = new FileSelect();
	publicPLine = new FileSelect();
	passwdLine = new QLineEdit();
	publicBox = new QGroupBox();
	saveAsLine = new FileSaveSelect();
	formBox = new QComboBox();
	typeBox = new QComboBox();
	xpathList = new QTextEdit();
	buttons = new QDialogButtonBox(this);

	buttons->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Abort);

	passwdLine->setEchoMode( QLineEdit::Password );
	passwdLine->setEnabled( false );

	formBox->addItem(QStringLiteral("Element"), XSec::EF_ELEMENT );
	formBox->addItem(QStringLiteral("Content"), XSec::EF_CONTENT );

	typeBox->addItem(QStringLiteral("AES128-CBC"),   XSec::EA_AES128_CBC );
	typeBox->addItem(QStringLiteral("AES192-CBC"), XSec::EA_AES192_CBC );
	typeBox->addItem(QStringLiteral("AES256-CBC"), XSec::EA_AES256_CBC );
	typeBox->addItem(QStringLiteral("3DES-CBC"), XSec::EA_3DES_CBC );

	upperLay->setFieldGrowthPolicy( QFormLayout::AllNonFixedFieldsGrow );
	upperLay->setLabelAlignment(Qt::AlignLeft);
	upperLay->setWidget(0, QFormLayout::LabelRole, formLabel);
	upperLay->setWidget(0, QFormLayout::FieldRole, formBox);
	upperLay->setWidget(1, QFormLayout::LabelRole, typeLabel);
	upperLay->setWidget(1, QFormLayout::FieldRole, typeBox);

	publicLay->setLabelAlignment(Qt::AlignLeft);
	publicLay->setWidget(0, QFormLayout::LabelRole, certRadio);
	publicLay->setWidget(0, QFormLayout::FieldRole, publicCLine);
	publicLay->setWidget(1, QFormLayout::LabelRole, keyRadio);
	publicLay->setWidget(1, QFormLayout::FieldRole, publicKLine);
	publicLay->setWidget(2, QFormLayout::LabelRole, p12Radio);
	publicLay->setWidget(2, QFormLayout::FieldRole, publicPLine);
	publicLay->setWidget(3, QFormLayout::LabelRole, passwdBox);
	publicLay->setWidget(3, QFormLayout::FieldRole, passwdLine);

	mainLay->addLayout(upperLay);
	mainLay->addLayout(publicLay);
	mainLay->addWidget(xpathLabel);
	mainLay->addWidget(xpathList);
	mainLay->addWidget(saveAsLabel);
	mainLay->addWidget(saveAsLine);
	mainLay->addWidget(buttons);

	connect(passwdBox, SIGNAL(toggled(bool)), this, SLOT(slotPasswdToggled(bool)));
	connect(certRadio, SIGNAL(toggled(bool)), this, SLOT(slotCertToggled(bool)));
	connect(keyRadio,  SIGNAL(toggled(bool)), this, SLOT(slotKeyToggled(bool)));
	connect(p12Radio,  SIGNAL(toggled(bool)), this, SLOT(slotP12Toggled(bool)));
	connect(buttons, SIGNAL(accepted()), this, SLOT(slotEncrypt()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	certRadio->setChecked(true);
	publicKLine->setEnabled(false);
	publicPLine->setEnabled(false);

	if(_file.isEmpty()){
		saveAsLine->setText(QStringLiteral("unbenannt-encrypted.xml"));
	} else {
		QFileInfo fi( _file );
		QString _newfile( "%1/%2-encrypted.xml" );
		saveAsLine->setText( _newfile.arg( fi.path()).arg( fi.baseName()));
	}
}

void XSDEncryptDialog::slotEncrypt() {
	XSec::encrypt_options_t eo;

	auto newfile = saveAsLine->text().toStdString();

	eo.encryption_form = formBox->currentData().toInt();
	eo.encryption_algorithm = typeBox->currentData().toInt();

	if( passwdBox->isChecked() )
		eo.key_password = passwdLine->text().toStdString();

	if( certRadio->isChecked() ){
		eo.public_key_is_cert = true;
		eo.public_key = publicCLine->text().toStdString();
	} else if (keyRadio->isChecked()) {
		eo.public_key_is_cert = false;
		eo.public_key = publicKLine->text().toStdString();
	} else {
		eo.keys_in_p12 = true;
		eo.public_key = publicPLine->text().toStdString();
	}

	for( auto &xp : xpathList->toPlainText().split("\n")) {
		eo.xpaths.push_back( xp.toStdString());
	}

	auto ret = core.encrypt( _file.toStdString(), newfile, eo );
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
