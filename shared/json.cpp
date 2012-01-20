/****************************************************************************
 **
 ** Original from QxtGore. Modified to use QByteArray for speed; 2012 <david.graeff@..web.de>
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtGore module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/


/*!
    \class QxtJSON
    \inmodule QxtCore
    \brief The QxtJSON class implements serializing/deserializing from/to JSON

    implements JSON (JavaScript Object Notation) is a lightweight data-interchange format. 
    see http://www.json.org/

    \section2 Type Conversion
    \table 80%
    \header \o JSON Type \o Qt Type
    \row  \o object \o QVariantMap/QVariantHash
    \row  \o array \o QVariantList/QStringList
    \row  \o string \o QByteArray
    \row  \o number \o int,double
    \row  \o true \o bool
    \row  \o false \o bool
    \row  \o null \o QVariant()

    \endtable

*/

#include "json.h"
#include <QVariant>
#include <QDebug>
#include <QTextStream>
#include <QStringList>

QByteArray JSON::stringify(const QVariant& v){
    if (v.isNull()){
        return "null";
    }
    int t = v.type();
    if (t == QVariant::String){
        return "\""+v.toByteArray()+"\"";
    }
    else if (t == QVariant::Bool){
        return v.toBool()?"true":"false";
    }else if (t ==  QVariant::Int){
        return QByteArray::number(v.toInt());
    }else if (t ==  QVariant::Double){
        return QByteArray::number(v.toDouble());
    }else if (t == QVariant::Map){
        QByteArray r="{";
        QMap<QString, QVariant> map = v.toMap();
        QMapIterator<QString, QVariant> i(map);
        while (i.hasNext()){
            i.next();
            r+="\""+i.key().toUtf8()+"\":"+stringify(i.value())+",";
        }
        if(r.length()>1)
            r.chop(1);
        r+="}";
        return r;
    }else if (t == QVariant::Hash){
        QByteArray r="{";
        QHash<QString, QVariant> map = v.toHash();
        QHashIterator<QString, QVariant> i(map);
        while (i.hasNext()){
            i.next();
            r+="\""+i.key().toUtf8()+"\":"+stringify(i.value())+",";
        }
        if(r.length()>1)
            r.chop(1);
        r+="}";
        return r;
    }else if (t == QVariant::StringList){
        QByteArray r="[";
        QStringList l = v.toStringList();
        foreach(QString i, l){
            r+="\""+i.toUtf8()+"\",";
        }
        if(r.length()>1)
            r.chop(1);
        r+="]";
        return r;
    }else if (t == QVariant::List){
        QByteArray r="[";
        QVariantList l = v.toList();
        foreach(QVariant i, l){
            r+=stringify(i)+",";
        }
        if(r.length()>1)
            r.chop(1);
        r+="]";
        return r;
    }

    return QByteArray();
}

//static QVariant parseValue(QTextStream &s,bool & error);
static QVariantMap parseObject (QTextStream & s,bool & error);
static QVariantList parseArray (QTextStream & s,bool & error);
static QByteArray parseString (QTextStream & s,bool & error);
static QVariant parseLiteral (QTextStream & s,bool & error);

QVariant JSON::parse(const QByteArray& string){
    QTextStream s(string);
    bool error=false;
    QVariant v=parseValue(s,error);
    if(error)
        return QVariant();
    return v;
}

QVariant JSON::parseValue(QTextStream &s,bool & error){
    s.skipWhiteSpace();
    char c;
    while(!s.atEnd() && !error){
        s>>c;
        if (c=='{'){
            return parseObject(s,error);
        } else if (c=='"'){
            return parseString(s,error);
        } else if (c=='['){
            return parseArray(s,error);
        } else {
            return parseLiteral(s,error);
        }
        s.skipWhiteSpace();
    }
    return QVariant();
}

static QVariantMap parseObject (QTextStream & s,bool & error){
    s.skipWhiteSpace();
    QVariantMap o;
    QByteArray key;
    bool atVal=false;

    char c;
    while(!s.atEnd() && !error){
        s>>c;
        if (c=='}'){
            return o;
        } else if (c==',' || c==':'){
            /*
              They're syntactic sugar, since key:value come in bundles anyway
              Could check for error handling. too lazy.
            */
        } else if (c=='"'){
            if(atVal){
                o[QString::fromUtf8(key)]=parseString(s,error);
                atVal=false;
            }else{
                key=parseString(s,error);
                atVal=true;
            }
        } else if (c=='['){
            if(atVal){
                o[QString::fromUtf8(key)]=parseArray(s,error);
                atVal=false;
            }else{
                error=true;
                return QVariantMap();
            }
        } else if (c=='{'){
            if(atVal){
                o[QString::fromUtf8(key)]=parseObject(s,error);
                atVal=false;
            }else{
                error=true;
                return QVariantMap();
            }
        } else {
            if(atVal){
                o[QString::fromUtf8(key)]=parseLiteral(s,error);
                atVal=false;
            }else{
                error=true;
                return QVariantMap();
            }
        }
        s.skipWhiteSpace();
    }
    error=true;
    return QVariantMap();
}
static QVariantList parseArray (QTextStream & s,bool & error){
    s.skipWhiteSpace();
    QVariantList l;
    char c;
    while(!s.atEnd() && !error){
        s>>c;
        if (c==']'){
            return l;
        } else if (c==','){
        } else if (c=='"'){
            l.append(parseString(s,error));
        } else if (c=='['){
            l.append(parseArray(s,error));
        } else if (c=='{'){
            l.append(parseObject(s,error));
        } else {
            l.append(parseLiteral(s,error));
        }
        s.skipWhiteSpace();
    }
    error=true;
    return QVariantList();
}
static QByteArray parseString (QTextStream & s,bool & error){
    QByteArray str;
    char c;
    while(!s.atEnd() && !error){
        s>>c;
        if(c=='"'){
            return str;
        }else if(c=='\\'){
            s>>c;
            if(c=='b'){
                str.append('\b');
            }else if(c=='f'){
                str.append('\f');
            }else if(c=='n'){
                str.append('\n');
            }else if(c=='r'){
                str.append('\r');
            }else if(c=='t'){
                str.append('\t');
            }else if(c=='f'){
                str.append('\f');
            }else if(c=='u'){
                short u;
                s>>u;
                str.append(char(u));
            }else{
                str.append(c);
            }
        }else{
            str.append(c);
        }
    }
    error=true;
    return QByteArray();
}
static QVariant parseLiteral (QTextStream & s,bool & error){
    s.seek(s.pos()-1);
    char c;
    while(!s.atEnd() && !error){
        s>>c;
        if (c=='t'){
            s>>c;//r
            s>>c;//u
            s>>c;//e
            return true;
        } else if (c=='f'){
            s>>c;//a
            s>>c;//l
            s>>c;//s
            s>>c;//e
            return false;
        }else if (c=='n'){
            s>>c;//u
            s>>c;//l
            s>>c;//l
            return QVariant();
        }else if (c=='-' || isdigit(c)){
            QByteArray n;
            while(( isdigit(c)  || (c=='.') || (c=='E') || (c=='e') || (c=='-') || (c=='+') )){
                n.append(c);
                if(s.atEnd() ||  error)
                    break;
                s>>c;
            }
            s.seek(s.pos()-1);
            if(n.contains('.')){
                return n.toDouble();
            }else{
                return n.toInt();
            }
        }
    }
    error=true;
    return QVariant();
}
