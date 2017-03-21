#ifndef SBFDATAMODEL_H
#define SBFDATAMODEL_H

#include <QStandardItemModel>
#include "sbfdataitem.h"

#include "sbfMesh.h"
#include "sbfReporter.h"

class SbfDataModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit SbfDataModel(QObject *parent = 0);

public:
    QList<SbfDataItem *> items(SbfDataItem::Type type = SbfDataItem::Type::Any) const;
    QStringList names(SbfDataItem::Type type = SbfDataItem::Type::Any) const;

    struct StepData {
        QString baseName;
        int numDigits;
        QString extention{".sba"};
        int curIndex;
        int minIndex;
        int maxIndex;
        StepData() = default;
        StepData(const QString &baseName, int numDigits, const QString &extention, int curIndex) :
            baseName(baseName),
            numDigits(numDigits),
            extention(extention),
            curIndex(curIndex),
            minIndex(curIndex),
            maxIndex(curIndex)
        {
            lookupMinMax();
        }
        void lookupMinMax()
        {
            NodesData<double> probe(0);
            probe.setName(baseName.toStdString().c_str());
            probe.setNumDigits(numDigits);
            probe.setStep(curIndex);
            if ( ! probe.exist() ) {
                curIndex = -1;
                minIndex = -1;
                maxIndex = -1;
                report.error("Cant deduce step files for ", baseName.toStdString());
                return;
            }
            minIndex = curIndex;
            while (true) {
                probe.setStep(minIndex-1);
                if(probe.exist())
                    minIndex--;
                else
                    break;
            }
            maxIndex = curIndex;
            while (true) {
                probe.setStep(maxIndex+1);
                if(probe.exist())
                    maxIndex++;
                else
                    break;
            }
            report(baseName.toStdString(), minIndex, curIndex, maxIndex);
        }
    };

    void addStepData(const QString &displayName,
                     const QString &baseName,
                     int numDigits,
                     const QString &extention,
                     int curIndex);

private:
    QMap<QString /*displayName*/, StepData> stepDataMap;

signals:

public slots:

};

#endif // SBFDATAMODEL_H
