#include "sbfdatamodel.h"

SbfDataModel::SbfDataModel(QObject *parent) :
    QStandardItemModel(parent)
{
}

QStringList SbfDataModel::names(SbfDataItem::Type type) const
{
    QStringList ns;
    auto its = items(type);
    for(const auto i : its) {
        ns << i->text();
    }
    return ns;
}

void SbfDataModel::addStepData(const QString &displayName, const QString &baseName, int numDigits, const QString &extention, int curIndex)
{
    stepDataMap[displayName] = StepData(baseName, numDigits, extention, curIndex);
}

QList<SbfDataItem *> SbfDataModel::items(SbfDataItem::Type type) const
{
    QList<SbfDataItem *> its;
    const int numRows = rowCount();
    for(int ct = 0; ct < numRows; ++ct) {
        auto item = dynamic_cast<SbfDataItem *>(this->item(ct, 0));
        if(item && (type == SbfDataItem::Type::Any || item->type() == type) )
            its << item;
    }
    return its;
}
