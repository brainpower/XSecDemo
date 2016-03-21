#ifndef XSD_DECRYPT_DIALOG_HPP
#define XSD_DECRYPT_DIALOG_HPP

#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QDialog>
#include <QRadioButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

#include "../lib/xseccore.hpp"
#include "xsd_reference_dialog.hpp"
#include "file_select.hpp"

class XSDDecryptDialog : public QDialog {
Q_OBJECT

public:

	explicit XSDDecryptDialog(const QString &file, QWidget *parent = 0);

	virtual ~XSDDecryptDialog() { }


public slots:
	QSize sizeHint() const;

	void slotPasswdToggled(bool c) { passwdLine->setEnabled(c); }

	void slotDecrypt();

private:
	void setupUi();

	XSec::Core core;
	QString _file;

	FileSelect *keyLine;
	QCheckBox *trustCert;
	QDialogButtonBox *buttons;
	QLabel *keyLabel, *saveAsLabel;
	QCheckBox *passwdBox;
	QLineEdit *passwdLine;

	QVBoxLayout *mainLay;
	QFormLayout *publicLay;
	FileSaveSelect *saveAsLine;

signals:
	void documentReady(const QUrl &);
};

#endif