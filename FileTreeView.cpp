#include "FileTreeView.h"
#include <QContextMenuEvent>
#include <QtWidgets/QFileSystemModel>
#include <QMessageBox>
#include <QDir>
#include <QJsonDocument>
#include <QTextStream>
#include <QTextCodec>
#include <QSqlQuery>
#include <QUuid>

#include <QtDebug>
#include <QSqlError>
#include "AddDirDlg.h"
#include "AddClassTypeDlg.h"
#include "DataType.h"
#include "AddClassRecordDlg.h"
#include "Document.h"
#include "CentralWidget.h"
#include "AddClassRecordsDlg.h"
#include "SQLSelectDlg.h"
#include "EditClassTypeDlg.h"

//ɾ���ļ���
bool deleteDir(const QString &path)
{
	if (path.isEmpty())
	{
		return false;
	}
	QDir dir(path);
	if (!dir.exists())
	{
		return true;
	}

	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); //���ù���
	QFileInfoList fileList = dir.entryInfoList(); // ��ȡ���е��ļ���Ϣ
	foreach(QFileInfo file, fileList)
	{ //�����ļ���Ϣ
		if (file.isFile())
		{ // ���ļ���ɾ��
			file.dir().remove(file.fileName());
		}
		else
		{ // �ݹ�ɾ��
			deleteDir(file.absoluteFilePath());
		}
	}

	return dir.rmdir(dir.absolutePath()); // ɾ���ļ���
}

bool classFile2JsonObj(QString filePath, QJsonObject& object)
{
	QFile file(filePath);

	if (file.exists())
	{
		QString strContent;
		if (file.open(QIODevice::ReadOnly))
		{
			QTextStream stream(&file);
			stream.setCodec(QTextCodec::codecForName("UTF-8"));
			strContent = stream.readAll();
		}

		file.close();

		if (!strContent.isNull() && !strContent.isEmpty())
		{
			QByteArray byteArray = strContent.toUtf8();

			QJsonParseError jsonErrorSrc;
			QJsonDocument doucmentSrc = QJsonDocument::fromJson(byteArray, &jsonErrorSrc);  // ת��Ϊ JSON �ĵ�
			if (!doucmentSrc.isNull() && (jsonErrorSrc.error == QJsonParseError::NoError))
			{  // ����δ��������
				if (doucmentSrc.isObject())
				{  // JSON �ĵ�Ϊ����
					object = doucmentSrc.object();  // ת��Ϊ����
					return true;
				}
			}
		}
	}

	return false;
}

FileTreeView::FileTreeView(QWidget *parent)
	: QTreeView(parent), m_pBuddy(nullptr)
{
	setWindowTitle(QString::fromLocal8Bit("���ݹ���"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	m_pModel = nullptr;

	m_pMenuOnDir = new QMenu(this);
	QAction* pActionAddDir = m_pMenuOnDir->addAction(QString::fromLocal8Bit("��ӷ���Ŀ¼"));
	QAction* pActionAddClass = m_pMenuOnDir->addAction(QString::fromLocal8Bit("�������"));
	//QAction* pActionDeleteDir = m_pMenuOnDir->addAction(QString::fromLocal8Bit("ɾ������Ŀ¼"));

	QAction* pActionAddClassTemplate = m_pMenuOnDir->addAction(QString::fromLocal8Bit("�������(ʹ���Ƽ�ģ��)"));

	connect(pActionAddDir, SIGNAL(triggered()), this, SLOT(slotAddDir()));
	connect(pActionAddClass, SIGNAL(triggered()), this, SLOT(slotAddClass()));
	connect(pActionAddClassTemplate, SIGNAL(triggered()), this, SLOT(slotAddClassUseTemplate()));
	//connect(pActionDeleteDir, SIGNAL(triggered()), this, SLOT(slotDeleteDir()));

	m_pMenuOnClass = new QMenu(this);
	QAction* pActionAddRecords = m_pMenuOnClass->addAction(QString::fromLocal8Bit("������Ӽ�¼"));
	QAction* pActionAddRecord = m_pMenuOnClass->addAction(QString::fromLocal8Bit("��Ӽ�¼"));
	QAction* pActionSearch = m_pMenuOnClass->addAction(QString::fromLocal8Bit("��ѯ"));
	QAction* pActionEditClass = m_pMenuOnClass->addAction(QString::fromLocal8Bit("�༭"));
	QAction* pActionDeleteClass = m_pMenuOnClass->addAction(QString::fromLocal8Bit("ɾ������"));

	connect(pActionAddRecord, SIGNAL(triggered()), this, SLOT(slotAddRecord()));
	connect(pActionSearch, SIGNAL(triggered()), this, SLOT(slotSearch()));
	connect(pActionEditClass, SIGNAL(triggered()), this, SLOT(slotEditClass()));
	connect(pActionAddRecords, SIGNAL(triggered()), this, SLOT(slotAddRecords()));
	connect(pActionDeleteClass, SIGNAL(triggered()), this, SLOT(slotDeleteClass()));

	m_pMenuOnEmpty = new QMenu(this);
	QAction* pActionAddDir2 = m_pMenuOnEmpty->addAction(QString::fromLocal8Bit("��ӷ���Ŀ¼"));
	//QAction* pActionAddClass2 = m_pMenuOnEmpty->addAction(QString::fromLocal8Bit("�������"));

	connect(pActionAddDir2, SIGNAL(triggered()), this, SLOT(slotAddDirOnEmpty()));

}

FileTreeView::~FileTreeView()
{

}

void FileTreeView::setWorkSpace(QString strWorkSpace)
{
	if (model() == nullptr)
	{
		m_pModel = new QFileSystemModel;
		setModel(m_pModel);
	}
	else
	{
		m_pModel = (QFileSystemModel*)model();
	}

	//m_pModel = new QFileSystemModel;
	m_pModel->setRootPath(strWorkSpace);
	m_pModel->setNameFilterDisables(false);
	QStringList strList;
	strList << "*.class";
	m_pModel->setNameFilters(strList);
	//setModel(m_pModel);
	setRootIndex(m_pModel->index(strWorkSpace));

	setColumnWidth(0, 400);
}

void FileTreeView::setBuddy(CentralWidget* pWidget)
{
	m_pBuddy = pWidget;
}

void FileTreeView::slotEditClass()
{
	QFileSystemModel* pModel = dynamic_cast<QFileSystemModel*>(model());
	QString strFilePath = pModel->filePath(m_currentIndex);

	QJsonObject object;
	if (classFile2JsonObj(strFilePath, object))
	{
		EditClassTypeDlg dlg(object);
		if (dlg.exec())
		{
			QJsonObject json = dlg.jsonObject();

			//ĿǰEditClassTypeDlgֻ֧������ֶΡ���Ϊsqlite��֧�֡�drop column��
			{
				//��ȡ����ӵ��ֶ�
				QJsonArray oldField = object.value(Class_Fields).toArray();
				QString strTableName = object.value(Class_Guid).toString();
				QJsonArray newField = json.value(Class_Fields).toArray();
				int nOldColumn = oldField.size();
				int nNewColumn = newField.size();

				for (int i = nOldColumn; i < nNewColumn; i ++)
				{
					QString strType;

					QJsonObject field = newField.at(i).toObject();
					QString dataType = field.value(Field_DataType).toString();
					QString fieldName = field.value(Field_Name).toString();

					if (dataType == FieldType_Int_Des)
					{
						strType = "integer";
					}
					else if (dataType == FieldType_Double_Des)
					{
						strType = "real";
					}
					else if (dataType == FieldType_Bool_Des)
					{
						strType = "boolean";
					}
					else if (dataType == FieldType_String_Des)
					{
						strType = "text";
					}
					else if (dataType == FieldType_File_Des)
					{
						strType = "text";
					}
					else if (dataType == FieldType_DateTime_Des)
					{
						strType = "datetime";
					}

					QString strSQL = "alter table " + strTableName + " add " + fieldName + " " + strType + " not null default 0";

					QSqlQuery sql_query;
					sql_query.prepare(strSQL);
					if (!sql_query.exec())
					{
						qDebug() << "Error: Fail to alter table." << sql_query.lastError();
					}
					else
					{
						qDebug() << "Table altered!";
					}
				}
			}

			QJsonDocument document;
			document.setObject(json);
			QByteArray byteArray = document.toJson(QJsonDocument::Indented);
			QString strContent(byteArray);

			QFileSystemModel* pModel = dynamic_cast<QFileSystemModel*>(model());
			QString strPath = pModel->filePath(m_currentIndex);

			QString strFileName = json.value(Class_Anno).toString();

			QFile file(strPath);
			if (file.open(QIODevice::WriteOnly))
			{
				QTextStream stream(&file);
				stream.setCodec(QTextCodec::codecForName("UTF-8"));
				stream << strContent;
			}

			QFileInfo fileInfo(strFilePath);
			m_pBuddy->showDBTable(fileInfo.path(), json);
		}
	}
}

void FileTreeView::slotAddDir()
{
	AddDirDlg dlg;
	if (dlg.exec())
	{
		QString strDirAdded = dlg.strDir();
		QFileSystemModel* pModel = dynamic_cast<QFileSystemModel*>(model());

		QString strPath = pModel->filePath(m_currentIndex);
// 		strPath += "/";
// 		strPath += strDirAdded;

		QDir dir(strPath);
		dir.mkdir(strDirAdded);
	}
}

void FileTreeView::slotAddDirOnEmpty()
{
	if (!Document::isProjectValid())
		return;

	AddDirDlg dlg;
	if (dlg.exec())
	{
		QString strDirAdded = dlg.strDir();
		QDir dir(Document::typeDir());
		dir.mkdir(strDirAdded);
	}
}

void FileTreeView::slotDeleteDir()
{
	QMessageBox messageBox;
	messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	messageBox.setWindowTitle(QString::fromLocal8Bit("ɾ��"));
	messageBox.setText(QString::fromLocal8Bit("�Ƿ�ɾ��Ŀ¼��"));
	
	int ret = messageBox.exec();
	if (ret == QMessageBox::Ok)
	{
		QFileSystemModel* pModel = dynamic_cast<QFileSystemModel*>(model());
		QString strPath = pModel->filePath(m_currentIndex);

		deleteDir(strPath);
	}
}

void FileTreeView::slotAddClass()
{
	AddClassTypeDlg dlg;
	if (dlg.exec())
	{
		QJsonObject json = dlg.jsonObject();
		QJsonDocument document;
		document.setObject(json);
		QByteArray byteArray = document.toJson(QJsonDocument::Indented);
		QString strContent(byteArray);

		QFileSystemModel* pModel = dynamic_cast<QFileSystemModel*>(model());
		QString strPath = pModel->filePath(m_currentIndex);

		QString strFileName = json.value(Class_Anno).toString();

		QString distanceFilePath = strPath + "/" + strFileName + ".class";
		QFile file(distanceFilePath);
		if (file.open(QIODevice::WriteOnly))
		{
			QTextStream stream(&file);
			stream.setCodec(QTextCodec::codecForName("UTF-8"));
			stream << strContent;
		}
	}
}

void FileTreeView::slotAddClassUseTemplate()
{
	QJsonObject jsonObjTemplate;

	QJsonArray jsonArray;
	{
		QJsonObject jsonObj1;
		jsonObj1.insert(Field_Name, QString::fromLocal8Bit("RCS�ļ�"));
		jsonObj1.insert(Field_DataType, "File");
		jsonArray.append(jsonObj1);
	}

	{
		QJsonObject jsonObj1;
		jsonObj1.insert(Field_Name, QString::fromLocal8Bit("RCSԭʼ�ļ�"));
		jsonObj1.insert(Field_DataType, "File");
		jsonArray.append(jsonObj1);
	}

	{
		QJsonObject jsonObj1;
		jsonObj1.insert(Field_Name, QString::fromLocal8Bit("ɢ��ϵ���ļ�"));
		jsonObj1.insert(Field_DataType, "File");
		jsonArray.append(jsonObj1);
	}

	{
		QJsonObject jsonObj1;
		jsonObj1.insert(Field_Name, QString::fromLocal8Bit("SARͼ���ļ�"));
		jsonObj1.insert(Field_DataType, "File");
		jsonArray.append(jsonObj1);
	}

	{
		QJsonObject jsonObj1;
		jsonObj1.insert(Field_Name, QString::fromLocal8Bit("��Ƶ�ļ�"));
		jsonObj1.insert(Field_DataType, "File");
		jsonArray.append(jsonObj1);
	}

	{
		QJsonObject jsonObj1;
		jsonObj1.insert(Field_Name, QString::fromLocal8Bit("����"));
		jsonObj1.insert(Field_DataType, "DateTime");
		jsonArray.append(jsonObj1);
	}

	{
		QJsonObject jsonObj1;
		jsonObj1.insert(Field_Name, QString::fromLocal8Bit("����Ƶ��"));
		jsonObj1.insert(Field_DataType, "Double");
		jsonArray.append(jsonObj1);
	}

	{
		QJsonObject jsonObj1;
		jsonObj1.insert(Field_Name, QString::fromLocal8Bit("����"));
		jsonObj1.insert(Field_DataType, "Double");
		jsonArray.append(jsonObj1);
	}

	{
		QJsonObject jsonObj1;
		jsonObj1.insert(Field_Name, QString::fromLocal8Bit("����"));
		jsonObj1.insert(Field_DataType, FieldType_Polarity_Des);
		jsonArray.append(jsonObj1);
	}

	{
		QJsonObject jsonObj1;
		jsonObj1.insert(Field_Name, QString::fromLocal8Bit("�Ƕ�"));
		jsonObj1.insert(Field_DataType, "Double");
		jsonArray.append(jsonObj1);
	}

	{
		QJsonObject jsonObj1;
		jsonObj1.insert(Field_Name, QString::fromLocal8Bit("�۲�ģʽ"));
		jsonObj1.insert(Field_DataType, FieldType_OBMode_Des);
		jsonArray.append(jsonObj1);
	}

	{
		QJsonObject jsonObj1;
		jsonObj1.insert(Field_Name, QString::fromLocal8Bit("�۲�Ŀ��"));
		jsonObj1.insert(Field_DataType, FieldType_OBTarget_Des);
		jsonArray.append(jsonObj1);
	}

	QUuid id = QUuid::createUuid();
	QString strId = id.toString();
	strId.remove(QChar('{'));
	strId.remove(QChar('}'));
	strId.remove(QChar('-'));
	jsonObjTemplate.insert(Class_Fields, jsonArray);
	jsonObjTemplate.insert(Class_Guid, "t" + strId);

	EditClassTypeDlg dlg(jsonObjTemplate, true);
	if (dlg.exec())
	{
		QJsonObject json = dlg.jsonObject();
		QJsonDocument document;
		document.setObject(json);
		QByteArray byteArray = document.toJson(QJsonDocument::Indented);
		QString strContent(byteArray);

		QFileSystemModel* pModel = dynamic_cast<QFileSystemModel*>(model());
		QString strPath = pModel->filePath(m_currentIndex);

		QString strFileName = json.value(Class_Anno).toString();

		QString distanceFilePath = strPath + "/" + strFileName + ".class";
		QFile file(distanceFilePath);
		if (file.open(QIODevice::WriteOnly))
		{
			QTextStream stream(&file);
			stream.setCodec(QTextCodec::codecForName("UTF-8"));
			stream << strContent;
		}
	}
}

void FileTreeView::slotDeleteClass()
{
	QMessageBox messageBox;
	messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	messageBox.setWindowTitle(QString::fromLocal8Bit("ɾ��"));
	messageBox.setText(QString::fromLocal8Bit("������������������ص�����"));

	int ret = messageBox.exec();
	if (ret != QMessageBox::Ok)
		return;

	m_pBuddy->showBlank();

	QFileSystemModel* pModel = dynamic_cast<QFileSystemModel*>(model());
	QString strFilePath = pModel->filePath(m_currentIndex);
	Document::deleteClass(strFilePath);
}

void FileTreeView::slotAddRecord()
{
	QFileSystemModel* pModel = dynamic_cast<QFileSystemModel*>(model());
	QString strFilePath = pModel->filePath(m_currentIndex);

	QJsonObject object;
	if (classFile2JsonObj(strFilePath, object))
	{
		AddClassRecordDlg addClassRecordDlg(object);
		if (addClassRecordDlg.exec())
		{
			const QJsonArray& record = addClassRecordDlg.recordJson();
			if (Document::addRecord(strFilePath, object, record))
			{
				QMessageBox box;
				box.setWindowTitle(QString::fromLocal8Bit("��ʾ"));
				box.setText(QString::fromLocal8Bit("��ӳɹ�"));
				box.exec();
			}
		}
	}

	QFileInfo fileInfo(strFilePath);
	m_pBuddy->showDBTable(fileInfo.path(), object);
}

void FileTreeView::slotAddRecords()
{
	QFileSystemModel* pModel = dynamic_cast<QFileSystemModel*>(model());
	QString strFilePath = pModel->filePath(m_currentIndex);

	QJsonObject object;
	if (classFile2JsonObj(strFilePath, object))
	{
		AddClassRecordsDlg dlg(strFilePath, object);
		if (dlg.exec())
		{
			QJsonArray records = dlg.recordsJson();
			for (int i = 0; i < records.size(); i ++)
			{
				QJsonArray record = records.at(i).toArray();
				if (!Document::addRecord(strFilePath, object, record))
				{
					QMessageBox box;
					box.setWindowTitle(QString::fromLocal8Bit("��ʾ"));
					box.setText(QString::fromLocal8Bit("���ʧ��"));
					box.exec();
				}
			}

			QMessageBox box;
			box.setWindowTitle(QString::fromLocal8Bit("��ʾ"));
			box.setText(QString::fromLocal8Bit("��ӳɹ�"));
			box.exec();
		}
	}

	QFileInfo fileInfo(strFilePath);
	m_pBuddy->showDBTable(fileInfo.path(), object);
}

void FileTreeView::slotSearch()
{
	QFileSystemModel* pModel = dynamic_cast<QFileSystemModel*>(model());
	QString strFilePath = pModel->filePath(m_currentIndex);

	if (strFilePath.isEmpty())
		return;

	QJsonObject object;
	if (classFile2JsonObj(strFilePath, object))
	{
		SQLSelectDlg dlg(object);
		if (dlg.exec())
		{
			QFileInfo fileInfo(strFilePath);
			QString strSQL = dlg.SQLString();
			m_pBuddy->showSelectDBTable(fileInfo.path(), object, strSQL);
		}
	}
}

void FileTreeView::contextMenuEvent(QContextMenuEvent *event)
{
	if (!Document::isProjectValid())
		return;

	QFileSystemModel* pModel = dynamic_cast<QFileSystemModel*>(model());
	m_currentIndex = indexAt(event->pos());

	if (m_currentIndex.isValid())
	{
		if (pModel->isDir(m_currentIndex))
		{
			m_pMenuOnDir->exec(event->globalPos());
		}
		else
		{
			m_pMenuOnClass->exec(event->globalPos());
		}
	}
	else
	{
		m_pMenuOnEmpty->exec(event->globalPos());
	}
}

void FileTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
	QFileSystemModel* pModel = dynamic_cast<QFileSystemModel*>(model());
	m_currentIndex = indexAt(event->pos());

	if (m_currentIndex.isValid())
	{
		QString strFilePath = pModel->filePath(m_currentIndex);
		if (strFilePath.isEmpty())
			return;

		QJsonObject object;
		if (classFile2JsonObj(strFilePath, object))
		{
			QFileInfo fileInfo(strFilePath);
			m_pBuddy->showDBTable(fileInfo.path(), object);
		}
	}
}