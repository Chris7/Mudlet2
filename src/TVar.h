#ifndef TVAR_H
#define TVAR_H

#include <QString>
#include <QStringList>
#include <QList>

class TVar
{
public:
    TVar();
    TVar(TVar *);
    TVar(TVar *, QString, int, QString, int);
    void    addChild(TVar *);
    void    setParent(TVar *);
    void    removeChild(TVar *);
    bool    setValue(QString);
    bool    setValue(QString, int);
    bool    setValueType(int);
    bool    setKeyType(int);
    bool    setName(QString);
    bool    setName(QString, int);
    void    setNewName(QString, int);
    void    setNewName(QString);
    QList<TVar *>  getChildren();
    TVar *  getParent();
    QString getValue();
    QString getName();
    QString getNewName();
    void clearNewName();
    int     getKeyType();
    int     getNewKeyType();
    int     getValueType();
public:
    bool    hidden;
private:
    QList<TVar *> children;
    TVar * parent;
    QString     name;
    int         kType;
    QString     value;
    int         vType;
    int         nkType;
    int         nvType;
    QString     nName;
    QString     nValue;
};

#endif // TVAR_H