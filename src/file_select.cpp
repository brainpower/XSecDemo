
#include "file_select.hpp"

#include <QFileDialog>
#include <QFileInfo>

FileSelect::FileSelect(QWidget *parent) : QWidget(parent){
	lineEdit   = new QLineEdit(this);
	pushButton = new QPushButton(QStringLiteral("...") , this);

	hlay = new QHBoxLayout(this);
	hlay->setContentsMargins(0,0,0,0);

	pushButton->setMaximumWidth(35);
	pushButton->setMinimumWidth(25);

	hlay->addWidget(lineEdit);
	hlay->addWidget(pushButton);

	connect(pushButton, SIGNAL(clicked()), this, SLOT(pushButtonAction()));
}

void FileSelect::pushButtonAction(){
	QString fn = QFileDialog::getOpenFileName(this, QStringLiteral("Open File..."), QFileInfo(lineEdit->text()).path());
	if(!fn.isEmpty()){
		lineEdit->setText(fn);
		emit fileChanged();
	}
}

QString FileSelect::text() const {
	return lineEdit->text();
}

void FileSelect::setText(const QString &t){
	lineEdit->setText(t);
}


void FileSelect::setReadOnly(bool r){
	lineEdit->setReadOnly(r);
}


void FileSelect::reset(){
	lineEdit->setText("");
}

FileSaveSelect::FileSaveSelect(QWidget *parent) : QWidget(parent){
	lineEdit   = new QLineEdit(this);
	pushButton = new QPushButton(QStringLiteral("...") , this);

	hlay = new QHBoxLayout(this);
	hlay->setContentsMargins(0,0,0,0);

	pushButton->setMaximumWidth(35);
	pushButton->setMinimumWidth(25);

	hlay->addWidget(lineEdit);
	hlay->addWidget(pushButton);

	connect(pushButton, SIGNAL(clicked()), this, SLOT(pushButtonAction()));
}

void FileSaveSelect::pushButtonAction(){
	QString fn = QFileDialog::getSaveFileName(this, QStringLiteral("Save File..."), QFileInfo(lineEdit->text()).path());
	if(!fn.isEmpty()){
		lineEdit->setText(fn);
		emit fileChanged();
	}
}

QString FileSaveSelect::text() const {
	return lineEdit->text();
}

void FileSaveSelect::setText(const QString &t){
	lineEdit->setText(t);
}


void FileSaveSelect::setReadOnly(bool r){
	lineEdit->setReadOnly(r);
}


void FileSaveSelect::reset(){
	lineEdit->setText("");
}

DirSelect::DirSelect(QWidget *parent) : QWidget(parent){
	lineEdit   = new QLineEdit(this);
	pushButton = new QPushButton(QStringLiteral("...") , this);

	hlay = new QHBoxLayout(this);
	hlay->setContentsMargins(0,0,0,0);
	pushButton->setMaximumWidth(35);
	pushButton->setMinimumWidth(25);

	hlay->addWidget(lineEdit);
	hlay->addWidget(pushButton);

	connect(pushButton, SIGNAL(clicked()), this, SLOT(pushButtonAction()));
}

void DirSelect::pushButtonAction(){
	QString fn = QFileDialog::getExistingDirectory(this, QStringLiteral("Open directory..."), QFileInfo(lineEdit->text()).path());
	if(!fn.isEmpty()){
		lineEdit->setText(fn);
		emit fileChanged();
	}
}

QString DirSelect::text() const {
	return lineEdit->text();
}

void DirSelect::setText(const QString &t){
	lineEdit->setText(t);
}


void DirSelect::setReadOnly(bool r){
	lineEdit->setReadOnly(r);
}

/*void DirSelect::setEnabled(bool e){
	lineEdit->setEnabled(e);
	pushButton->setEnabled(e);
}*/

void DirSelect::reset(){
	lineEdit->setText("");
}