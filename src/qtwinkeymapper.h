#ifndef QTWINKEYMAPPER_H
#define QTWINKEYMAPPER_H

#include <QObject>
#include <QHash>
#include <QChar>

#include "qtkeymapperbase.h"

class QtWinKeyMapper : public QtKeyMapperBase
{
    Q_OBJECT
public:
    explicit QtWinKeyMapper(QObject *parent = 0);

    virtual unsigned int returnQtKey(unsigned int key, unsigned int scancode=0);

protected:
    void populateMappingHashes();
    void populateCharKeyInformation();

signals:

public slots:

};

#endif // QTWINKEYMAPPER_H
