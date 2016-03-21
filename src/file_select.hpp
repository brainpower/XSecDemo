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


#ifndef FILE_SELECT_HPP
#define FILE_SELECT_HPP

#include <QWidget>
#include <QLineEdit>
#include <QString>
#include <QPushButton>
#include <QHBoxLayout>

class FileSelect : public QWidget{
	Q_OBJECT
public:
	FileSelect(QWidget *parent=0);

	QString text() const;

	void setReadOnly(bool r);
	/*void setEnabled(bool e);*/
	void setText(const QString& t);

	QLineEdit *line(){ return lineEdit; }
public slots:
	void reset();

private:
	QLineEdit   *lineEdit;
	QPushButton *pushButton;
	QHBoxLayout *hlay;

private slots:
	void pushButtonAction();

signals:
	void fileChanged();
};
class FileSaveSelect : public QWidget{
Q_OBJECT
public:
	FileSaveSelect(QWidget *parent=0);

	QString text() const;

	void setReadOnly(bool r);
	/*void setEnabled(bool e);*/
	void setText(const QString& t);

	QLineEdit *line(){ return lineEdit; }
public slots:
	void reset();

private:
	QLineEdit   *lineEdit;
	QPushButton *pushButton;
	QHBoxLayout *hlay;

private slots:
	void pushButtonAction();

signals:
	void fileChanged();
};
class DirSelect : public QWidget{
	Q_OBJECT
public:
	DirSelect(QWidget *parent=0);

	QString text() const;

	/*void setEnabled(bool e);*/
	void setReadOnly(bool r);
	void setText(const QString& t);

	QLineEdit *line(){ return lineEdit; }
public slots:
	void reset();

private:
	QLineEdit   *lineEdit;
	QPushButton *pushButton;
	QHBoxLayout *hlay;

private slots:
	void pushButtonAction();

signals:
	void fileChanged();
};


#endif