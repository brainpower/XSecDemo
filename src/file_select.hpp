
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