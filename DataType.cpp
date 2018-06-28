#include "DataType.h"

QVector<QString> DataType::m_vecFieldDescription;

QVector<QString> DataType::m_vecOBModeDescription;
QVector<QString> DataType::m_vecOBTargetDescription;
QVector<QString> DataType::m_vecgPolarityDescription;

QString g_strOBMode = QString::fromLocal8Bit("�۲�ģʽ");
QString g_strOBTarget = QString::fromLocal8Bit("�۲�Ŀ��");
QString g_strPolarity = QString::fromLocal8Bit("����");

DataType::DataType(QObject *parent)
	: QObject(parent)
{

}

DataType::~DataType()
{

}

void DataType::initEnumType()
{
	if (m_vecOBModeDescription.isEmpty())
	{
		m_vecOBModeDescription.push_back(QString::fromLocal8Bit("����"));
		m_vecOBModeDescription.push_back(QString::fromLocal8Bit("�̶�������"));
	}

	if (m_vecOBTargetDescription.isEmpty())
	{
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("��ͨ����"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("���齢"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("�б�"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("ս��"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("���Ӳ�"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("���Ӳ�"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("������"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("��ĸ"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("����"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("�д�"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("Ǳͧ"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("̹��"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("���䳵"));
	}

	if (m_vecgPolarityDescription.isEmpty())
	{
		m_vecgPolarityDescription.push_back("HH");
		m_vecgPolarityDescription.push_back("HV");
		m_vecgPolarityDescription.push_back("VH");
		m_vecgPolarityDescription.push_back("VV");
	}
}

int DataType::typeCount()
{
	return MaxCount;
}

QString DataType::typeDescription(int nIndex)
{
	if (m_vecFieldDescription.isEmpty())
	{
		m_vecFieldDescription.push_back(FieldType_Int_Des);
		m_vecFieldDescription.push_back(FieldType_Double_Des);
		m_vecFieldDescription.push_back(FieldType_Bool_Des);
		m_vecFieldDescription.push_back(FieldType_String_Des);
		m_vecFieldDescription.push_back(FieldType_File_Des);
		m_vecFieldDescription.push_back(FieldType_DateTime_Des);
		m_vecFieldDescription.push_back(FieldType_OBMode_Des);
		m_vecFieldDescription.push_back(FieldType_OBTarget_Des);
		m_vecFieldDescription.push_back(FieldType_Polarity_Des);
	}

	if (nIndex < 0 || nIndex >= MaxCount)
	{
		return "";
	}
	else
	{
		return m_vecFieldDescription[nIndex];
	}
}

int DataType::enumTypeCount(const QString& enumName)
{
	initEnumType();

	if (enumName.compare(FieldType_OBMode_Des, Qt::CaseInsensitive) == 0)
	{
		return m_vecOBModeDescription.size();
	}
	else if (enumName.compare(FieldType_OBTarget_Des, Qt::CaseInsensitive) == 0)
	{
		return m_vecOBTargetDescription.size();
	}
	else if (enumName.compare(FieldType_Polarity_Des, Qt::CaseInsensitive) == 0)
	{
		return m_vecgPolarityDescription.size();
	}

	return 0;
}

QString DataType::enumTypeDescription(const QString& enumName, int nIndex)
{
	initEnumType();

	if (enumName.compare(FieldType_OBMode_Des, Qt::CaseInsensitive) == 0)
	{
		if (nIndex < 0 || nIndex >= m_vecOBModeDescription.size())
			return "";

		return m_vecOBModeDescription[nIndex];
	}
	else if (enumName.compare(FieldType_OBTarget_Des, Qt::CaseInsensitive) == 0)
	{
		if (nIndex < 0 || nIndex >= m_vecOBTargetDescription.size())
			return "";

		return m_vecOBTargetDescription[nIndex];
	}
	else if (enumName.compare(FieldType_Polarity_Des, Qt::CaseInsensitive) == 0)
	{
		if (nIndex < 0 || nIndex >= m_vecgPolarityDescription.size())
			return "";

		return m_vecgPolarityDescription[nIndex];
	}

	return "";
}