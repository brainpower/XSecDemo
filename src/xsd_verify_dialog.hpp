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