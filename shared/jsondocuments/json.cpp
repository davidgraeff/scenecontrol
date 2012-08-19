/****************************************************************************
** Copyright (c) 2006 - 2011, the LibQxt project.
** See the Qxt AUTHORS file for a list of authors and copyright holders.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the LibQxt project nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS QLatin1String("AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
** <http://libqxt.org>  <foundation@libqxt.org>
*****************************************************************************/


/*!
    \class JSON
    \inmodule QxtCore
    \brief The JSON class implements serializing/deserializing from/to JSON

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

QString JSON::stringify(QVariant v){
    if (v.isNull()){
        return QLatin1String("null");
    }
    switch (v.type()) {
        case QVariant::Bool:
            return v.toBool()?QLatin1String("true"):QLatin1String("false");
            break;
        case QVariant::ULongLong:
        case QVariant::UInt:
            return QString::number(v.toULongLong());
            break;
        case QVariant::LongLong:
        case QVariant::Int:
            return QString::number(v.toLongLong());
            break;
        case QVariant::Double:
            return QString::number(v.toDouble());
            break;
        case QVariant::Map:
            {
                QString r=QLatin1String("{");
                QMap<QString, QVariant> map = v.toMap();
                QMapIterator<QString, QVariant> i(map);
                while (i.hasNext()){
                    i.next();
                    r+=QLatin1String("\"")+i.key()+ QLatin1String("\":") +stringify(i.value())+QLatin1String(",");
                }
                if(r.length()>1)
                    r.chop(1);
                r+=QLatin1String("}");
                return r;
            }
            break;
#if QT_VERSION >= 0x040500
        case QVariant::Hash:
            {
                QString r=QLatin1String("{");
                QHash<QString, QVariant> map = v.toHash();
                QHashIterator<QString, QVariant> i(map);
                while (i.hasNext()){
                    i.next();
                    r+=QLatin1String("\"")+i.key()+QLatin1String("\":")+stringify(i.value())+QLatin1String(",");
                }
                if(r.length()>1)
                    r.chop(1);
                r+=QLatin1String("}");
                return r;
            }
            break;
#endif
        case QVariant::StringList:
            {
                QString r=QLatin1String("[");
                QStringList l = v.toStringList();
                foreach(QString i, l){
                    r+=QLatin1String("\"")+i+QLatin1String("\",");
                }
                if(r.length()>1)
                    r.chop(1);
                r+=QLatin1String("]");
                return r;
            }
        case QVariant::List:
            {
                QString r=QLatin1String("[");
                QVariantList l = v.toList();
                foreach(QVariant i, l){
                    r+=stringify(i)+QLatin1String(",");
                }
                if(r.length()>1)
                    r.chop(1);
                r+=QLatin1String("]");
                return r;
            }
            break;
        case QVariant::String:
        default:
            {
                QString in = v.toString();
                QString out;
                for(QString::ConstIterator i = in.constBegin(); i != in.constEnd(); i++){
                    if( (*i) == QLatin1Char('\b'))
                        out.append(QLatin1String("\\b"));
                    else if( (*i) == QLatin1Char('\f'))
                        out.append(QLatin1String("\\f"));
                    else if( (*i) == QLatin1Char('\n'))
                        out.append(QLatin1String("\\n"));
                    else if( (*i) == QLatin1Char('\r'))
                        out.append(QLatin1String("\\r"));
                    else if( (*i) == QLatin1Char('\t'))
                        out.append(QLatin1String("\\t"));
                    else if( (*i) == QLatin1Char('\f'))
                        out.append(QLatin1String("\\f"));
                    else if( (*i) == QLatin1Char('\\'))
                        out.append(QLatin1String("\\\\"));
                    else if( (*i) == QLatin1Char('/'))
                        out.append(QLatin1String("\\/"));
                    else
                        out.append(*i);
                }
                return QLatin1String("\"")+out+QLatin1String("\"");
            }
            break;
    }
    return QString();
}

// static QVariant parseValue(QTextStream &s,bool & error);
static QVariantMap parseObject (QTextStream & s,bool & error);
static QVariantList parseArray (QTextStream & s,bool & error);
static QString parseString (QTextStream & s,bool & error);
static QVariant parseLiteral (QTextStream & s,bool & error);

QVariant JSON::parse(const QByteArray& string) {
  return parse(QString::fromUtf8(string));
}

QVariant JSON::parse(QString string){
    QTextStream s(&string);
    bool error=false;
    QVariant v=JSON::parseValue(s,error);
    if(error)
        return QVariant();
    return v;
}



QVariant JSON::parseValue(QTextStream &s,bool & error){
    s.skipWhiteSpace();
    QChar c;
    while(!s.atEnd() && !error){
        s>>c;
        if (c==QLatin1Char('{')){
            return parseObject(s,error);
        } else if (c==QLatin1Char('"')){
            return parseString(s,error);
        } else if (c==QLatin1Char('[')){
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
    QString key;
    bool atVal=false;

    QChar c;
    while(!s.atEnd() && !error){
        s>>c;
        if (c==QLatin1Char('}')){
            return o;
        } else if (c==QLatin1Char(',') || c==QLatin1Char(':')){
            /*
              They're syntactic sugar, since key:value come in bundles anyway
              Could check for error handling. too lazy.
            */
        } else if (c==QLatin1Char('"')){
            if(atVal){
                o[key]=parseString(s,error);
                atVal=false;
            }else{
                key=parseString(s,error);
                atVal=true;
            }
        } else if (c==QLatin1Char('[')){
            if(atVal){
                o[key]=parseArray(s,error);
                atVal=false;
            }else{
                error=true;
                return QVariantMap();
            }
        } else if (c==QLatin1Char('{')){
            if(atVal){
                o[key]=parseObject(s,error);
                atVal=false;
            }else{
                error=true;
                return QVariantMap();
            }
        } else {
            if(atVal){
                o[key]=parseLiteral(s,error);
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
    QChar c;
    while(!s.atEnd() && !error){
        s>>c;
        if (c==QLatin1Char(']')){
            return l;
        } else if (c==QLatin1Char(',')){
        } else if (c==QLatin1Char('"')){
            l.append(QVariant(parseString(s,error)));
        } else if (c==QLatin1Char('[')){
            l.append(QVariant(parseArray(s,error)));
        } else if (c==QLatin1Char('{')){
            l.append(QVariant(parseObject(s,error)));
        } else {
            l.append(QVariant(parseLiteral(s,error)));
        }
        s.skipWhiteSpace();
    }
    error=true;
    return QVariantList();
}
static QString parseString (QTextStream & s,bool & error){
    QString str;
    QChar c;
    while(!s.atEnd() && !error){
        s>>c;
        if(c==QLatin1Char('"')){
            return str;
        }else if(c==QLatin1Char('\\')){
            s>>c;
            if(c==QLatin1Char('b')){
                str.append(QLatin1Char('\b'));
            }else if(c==QLatin1Char('f')){
                str.append(QLatin1Char('\f'));
            }else if(c==QLatin1Char('n')){
                str.append(QLatin1Char('\n'));
            }else if(c==QLatin1Char('r')){
                str.append(QLatin1Char('\r'));
            }else if(c==QLatin1Char('t')){
                str.append(QLatin1Char('\t'));
            }else if(c==QLatin1Char('f')){
                str.append(QLatin1Char('\f'));
            }else if(c==QLatin1Char('u')){
                QString k;
                for (int i = 0; i < 4; i++ ) {
                    s >> c;
                    k.append(c);
                }
                bool ok;
                int i = k.toInt(&ok, 16);
                if (ok)
                    str.append(QLatin1Char(i));
            }else{
                str.append(c);
            }
        }else{
            str.append(c);
        }
    }
    error=true;
    return QString();
}
static QVariant parseLiteral (QTextStream & s,bool & error){
    s.seek(s.pos()-1);
    QChar c;
    while(!s.atEnd() && !error){
        s>>c;
        if (c==QLatin1Char('t')){
            s>>c;//r
            s>>c;//u
            s>>c;//e
            return true;
        } else if (c==QLatin1Char('f')){
            s>>c;//a
            s>>c;//l
            s>>c;//s
            s>>c;//e
            return false;
        }else if (c==QLatin1Char('n')){
            s>>c;//u
            s>>c;//l
            s>>c;//l
            return QVariant();
        }else if (c==QLatin1Char('-') || c.isDigit()){
            QString n;
            while(( c.isDigit()  || (c==QLatin1Char('.')) || (c==QLatin1Char('E')) || (c==QLatin1Char('e')) || (c==QLatin1Char('-')) || (c==QLatin1Char('+')) )){
                n.append(c);
                if(s.atEnd() ||  error)
                    break;
                s>>c;
            }
            s.seek(s.pos()-1);
            if(n.contains(QLatin1Char('.'))) {
                return n.toDouble();
            } else {
                bool ok = false;
                int result = n.toInt(&ok);
                if(ok) return result;
                return n.toLongLong();
            }
        }
    }
    error=true;
    return QVariant();
}
