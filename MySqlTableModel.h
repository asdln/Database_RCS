#ifndef MYSQLTABLEMODEL_H
#define MYSQLTABLEMODEL_H

#include <QSqlTableModel>
#include <QtCore/QJsonObject>

//����һ��model�����ڿ��ơ��ļ����ֶβ��ܱ��༭��
class MySqlTableModel : public QSqlTableModel
{
	Q_OBJECT

public:
	MySqlTableModel(QObject *parent);
	~MySqlTableModel();

	Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

	void SetJsonObj(const QJsonObject& currentObj);

private:

	QJsonObject m_currentObj;
	QMap<int, int> m_mapFileIndex;
};

#endif // MYSQLTABLEMODEL_H
