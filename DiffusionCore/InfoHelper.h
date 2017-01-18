#ifndef INFOHELPER_H
#define INFOHELPER_H

#include <QList>
#include <QVariant>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class TreeItem
{
public:
    explicit TreeItem(const QList<QVariant> &data, TreeItem *parentItem = 0)
	{
		m_parentItem = parentItem;
        m_itemData = data;
	};
    ~TreeItem()
	{
		 qDeleteAll(m_childItems);
	};

    void appendChild(TreeItem *child)
	{
		m_childItems.append(child);
	};

    TreeItem *child(int row)
	{
		return m_childItems.value(row);
	};
    int childCount() const
	{
		return m_childItems.count();
	};
    int columnCount() const
	{
		return m_itemData.count();
	};
    QVariant data(int column) const
	{
		return m_itemData.value(column);
	};
    int row() const
	{

		 if (m_parentItem)
			 return m_parentItem->m_childItems.indexOf(const_cast<TreeItem*>(this));
		 return 0;
	};
    TreeItem *parentItem()
	{
				 return m_parentItem;
	};

private:
    QList<TreeItem*> m_childItems;
    QList<QVariant> m_itemData;
    TreeItem *m_parentItem;
};

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
	TreeModel(const QStringList &headers, const QString &data,
		QObject *parent = 0);
	TreeModel(const QStringList &headers, QObject *parent = 0);
    ~TreeModel();

    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

	bool setData(const QModelIndex &index, const QVariant &value,
		int role = Qt::EditRole) Q_DECL_OVERRIDE;
	bool setHeaderData(int section, Qt::Orientation orientation,
		const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;

	bool insertColumns(int position, int columns,
		const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
	bool removeColumns(int position, int columns,
		const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
	bool insertRows(int position, int rows,
		const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
	bool removeRows(int position, int rows,
		const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;

private:
    void setupModelData(const QStringList &lines, TreeItem *parent);
	TreeItem *getItem(const QModelIndex &index) const;

    TreeItem *rootItem;
};

#endif // INFOHELPER_H
