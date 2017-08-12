#ifndef comboxkey_h__
#define comboxkey_h__

#pragma once

#include <QObject>

class IComboxKey 
{
    struct Items
    {
        Items(const QString& itm, const QString& o)
            :item(itm), val(o) {

        }

        QString item;   // ÃèÊö
        QString val; // ²Ù×÷Êý
    };

public:
    virtual ~IComboxKey(){

    }

    void fillCombobox(QComboBox* cmb){
        for (const Items& t : items_){
            cmb->addItem(t.item);
        }
    }

    QString getSelectedValue(QComboBox*cmb){
        QString sop = cmb->currentText();
        for (auto& t : items_){
            if (t.item == sop){
                return t.val;
            }
        }
        return QString::null;
    }

    void append(const QString& item, const QString& val = "") {
        items_.push_back(Items(item, val));
    }

protected:
    QList<Items> items_;
};

#endif // comboxkey_h__
