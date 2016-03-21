#ifndef XSD_ENCRYPT_DIALOG_HPP
#define XSD_ENCRYPT_DIALOG_HPP

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

class XSDEncryptDialog : public QDialog {
Q_OBJECT

public:

	explicit XSDEncryptDialog(const QString &file, QWidget *parent = 0);

	virtual ~XSDEncryptDialog() { }

public slots:
	QSize sizeHint() const;

	void slotPasswdToggled(bool c) { passwdLine->setEnabled(c); }
	void slotCertToggled(bool c) { publicCLine->setEnabled(c); }
	void slotKeyToggled(bool c) { publicKLine->setEnabled(c); }
	void slotP12Toggled(bool c) { publicPLine->setEnabled(c); }

	void slotEncrypt();

private:
	void setupUi();

	XSec::Core core;
	QString _file;

	QComboBox *typeBox, *formBox;
	QLabel *typeLabel, *formLabel, *xpathLabel, *saveAsLabel;
	FileSelect *publicKLine, *publicCLine, *publicPLine;
	FileSaveSelect *saveAsLine;
	QTextEdit *xpathList;
	QDialogButtonBox *buttons;
	QCheckBox *passwdBox;
	QLineEdit *passwdLine;
	QRadioButton *certRadio, *keyRadio, *p12Radio;
	QGroupBox *publicBox;

	QVBoxLayout *mainLay;
	QFormLayout *upperLay, *publicLay;

signals:
	void documentReady(const QUrl &);
};


#endif