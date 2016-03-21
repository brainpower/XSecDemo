#ifndef XSD_VERIFY_DIALOG
#define XSD_VERIFY_DIALOG

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

class XSDVerifyDialog : public QDialog {
Q_OBJECT

public:

	explicit XSDVerifyDialog(const QString &file, QWidget *parent = 0);

	virtual ~XSDVerifyDialog() {
		for( auto r : _refs ){
			delete r;
		}
	}


public slots:
	QSize sizeHint() const;
	void slotCertToggled(bool c) { publicCLine->setEnabled(c); trustCert->setEnabled(!c); }
	void slotKeyToggled(bool c)  { publicKLine->setEnabled(c); trustCert->setEnabled(!c); }

	void slotVerify();

private:
	void setupUi();

	XSec::Core core;
	std::vector<XSec::Reference*> _refs;
	QString _file;

	QGroupBox *publicBox;
	QRadioButton *certRadio, *keyRadio, *noneRadio;
	FileSelect *publicKLine, *publicCLine;
	QCheckBox *trustCert;
	QDialogButtonBox *buttons;
	QLabel *noteLabel;

	QVBoxLayout *mainLay;
	QFormLayout *publicLay;

};

#endif /* ifndef XSD_TRANSFORM_DIALOG */