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

#ifndef XSD_REFERENCE_DIALOG
#define XSD_REFERENCE_DIALOG

#include <QComboBox>
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

class XSDReferenceDialog : public QDialog {
Q_OBJECT

public:

	explicit XSDReferenceDialog(int type, QWidget *parent = 0);
	explicit XSDReferenceDialog(const XSec::Reference &ref, QWidget *parent = 0);
	virtual ~XSDReferenceDialog();

	XSec::Reference ref() const { return _ref; }
public slots:
	QSize sizeHint() const;

	void slotHashChanged() { _ref.hash  = hashBox->currentData().toInt(); }
	void slotURIChanged()  { _ref.uri   = uriLine->text().toStdString(); }
	void slotXPIChanged()   { _ref.xpath_intersect = xpathILine->text().toStdString(); }
	void slotXPSChanged()   { _ref.xpath_subtract  = xpathSLine->text().toStdString(); }
	void slotXPUChanged()   { _ref.xpath_union     = xpathULine->text().toStdString(); }
	void slotTransChanged(){ _ref.transform = transBox->currentData().toInt();	}
	void slotUriSelect();

private:
	void setupUi();

	XSec::Reference _ref;

	QComboBox *hashBox;
	QLabel *uriLabel, *hashLabel, *transLabel, *xpathILabel, *xpathSLabel, *xpathULabel;
	QLineEdit *uriLine, *xpathILine, *xpathSLine, *xpathULine;
	QComboBox *transBox;
	QDialogButtonBox *buttons;
	QPushButton *uriBtn;

	QVBoxLayout *mainLay;
	QHBoxLayout *uriLay;
	QFormLayout *upperLay;
};

#endif /* ifndef XSD_TRANSFORM_DIALOG */
