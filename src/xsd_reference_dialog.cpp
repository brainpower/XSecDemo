#include <lib/xseccore.hpp>
#include "xsd_reference_dialog.hpp"

#include <QFileDialog>

XSDReferenceDialog::XSDReferenceDialog(int type, QWidget *parent) {
	setupUi();

	_ref.hash = hashBox->currentData().toInt();
	_ref.transform = transBox->currentData().toInt();
}

XSDReferenceDialog::XSDReferenceDialog(const XSec::Reference &ref, QWidget *parent) {
	setupUi();

	_ref = ref;
	uriLine->setText( QString::fromStdString(_ref.uri) );
	hashBox->setCurrentIndex( _ref.hash - 1 ); // FIXME: set by userdata if possible
	transBox->setCurrentIndex( _ref.transform );
}

XSDReferenceDialog::~XSDReferenceDialog() {

}

QSize XSDReferenceDialog::sizeHint() const {
	return QSize(400, 300).expandedTo(minimumSizeHint());
}

void XSDReferenceDialog::setupUi() {
	setWindowTitle(QStringLiteral("Referenz"));

	mainLay = new QVBoxLayout(this);
	upperLay = new QFormLayout();

	uriLabel = new QLabel(QStringLiteral("URI"));
	hashLabel = new QLabel(QStringLiteral("Hash"));
	transLabel = new QLabel(QStringLiteral("Kanonisierung"));
	xpathILabel = new QLabel(QStringLiteral("XPath2 intersect"));
	xpathSLabel = new QLabel(QStringLiteral("XPath2 subtract"));
	xpathULabel = new QLabel(QStringLiteral("XPath2 union"));
	uriLine = new QLineEdit();
	xpathILine = new QLineEdit();
	xpathSLine = new QLineEdit();
	xpathULine = new QLineEdit();
	hashBox = new QComboBox();
	transBox = new QComboBox();
	uriBtn = new QPushButton(QStringLiteral("..."));
	uriLay = new QHBoxLayout();
	uriLay->setContentsMargins(0,0,0,0);
	buttons = new QDialogButtonBox(this);
	buttons->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Abort);

	uriBtn->setMinimumWidth(25);
	uriBtn->setMaximumWidth(35);

	hashBox->addItem(QStringLiteral("SHA1"), XSec::HA_SHA1 );
	hashBox->addItem(QStringLiteral("SHA224"), XSec::HA_SHA224 );
	hashBox->addItem(QStringLiteral("SHA256"), XSec::HA_SHA256 );
	hashBox->addItem(QStringLiteral("SHA384"), XSec::HA_SHA384 );
	hashBox->addItem(QStringLiteral("SHA512"), XSec::HA_SHA512 );

	transBox->addItem(QStringLiteral("Keine"), XSec::C14N_UNSET);
	transBox->addItem(QStringLiteral("inclusiv 1.1"), XSec::C14N_11_INCLUSIVE);
	transBox->addItem(QStringLiteral("inclusiv"), XSec::C14N_INCLUSIVE);
	transBox->addItem(QStringLiteral("exklusiv"), XSec::C14N_EXCLUSIVE);

	uriLay->addWidget(uriLine);
	uriLay->addWidget(uriBtn);

	upperLay->setLabelAlignment(Qt::AlignLeft);
	upperLay->setFieldGrowthPolicy( QFormLayout::AllNonFixedFieldsGrow );
	upperLay->setWidget(0, QFormLayout::LabelRole, uriLabel);
	upperLay->setLayout(0, QFormLayout::FieldRole, uriLay);
	upperLay->setWidget(1, QFormLayout::LabelRole, hashLabel);
	upperLay->setWidget(1, QFormLayout::FieldRole, hashBox);
	upperLay->setWidget(2, QFormLayout::LabelRole, transLabel);
	upperLay->setWidget(2, QFormLayout::FieldRole, transBox);
	upperLay->setWidget(3, QFormLayout::LabelRole, xpathILabel);
	upperLay->setWidget(3, QFormLayout::FieldRole, xpathILine);
	upperLay->setWidget(4, QFormLayout::LabelRole, xpathSLabel);
	upperLay->setWidget(4, QFormLayout::FieldRole, xpathSLine);
	upperLay->setWidget(5, QFormLayout::LabelRole, xpathULabel);
	upperLay->setWidget(5, QFormLayout::FieldRole, xpathULine);

	mainLay->addLayout(upperLay);
	mainLay->addWidget(buttons);

	connect(hashBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotHashChanged()));
	connect(uriLine, SIGNAL(textChanged(const QString&)), this, SLOT(slotURIChanged()));
	connect(xpathILine, SIGNAL(textChanged(const QString&)), this, SLOT(slotXPIChanged()));
	connect(xpathSLine, SIGNAL(textChanged(const QString&)), this, SLOT(slotXPSChanged()));
	connect(xpathULine, SIGNAL(textChanged(const QString&)), this, SLOT(slotXPUChanged()));
	connect(transBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotTransChanged()));
	connect(uriBtn, SIGNAL(clicked()), this, SLOT(slotUriSelect()));

	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
}

void XSDReferenceDialog::slotUriSelect() {
	QUrl fn = QFileDialog::getOpenFileUrl(this, QStringLiteral("Save File..."));
	if(!fn.isEmpty()){
		uriLine->setText(fn.toString());
	}
}
