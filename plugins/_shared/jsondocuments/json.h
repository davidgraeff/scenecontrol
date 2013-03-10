#pragma once
/****************************************************************************
 * *
 ** Copyright (c) 2010 Girish Ramakrishnan <girish@forwardbias.in>
 **
 ** Use, modification and distribution is allowed without limitation,
 ** warranty, liability or support of any kind.
 **
 ****************************************************************************/

#include <QByteArray>
#include <QVariant>
#include <QDebug>

class JsonReader
{
public:
	JsonReader();
	~JsonReader();
	
	bool parse(const QByteArray &ba);
	bool parse(const QString &str);
	
	QVariant result() const;
	
	QString errorString() const;
	
private:
	QVariant m_result;
	QString m_errorString;
	void *reserved;
};

class JsonWriter
{
public:
	JsonWriter();
	~JsonWriter();
	
	bool stringify(const QVariant &variant);
	
	QString result() const;
	
	QString errorString() const;
	
	void setAutoFormatting(bool autoFormat);
	bool autoFormatting() const;
	
	void setAutoFormattingIndent(int spaceOrTabs);
	int autoFormattingIndent() const;
	
private:
	void stringify(const QVariant &variant, int depth);
	
	QString m_result;
	QString m_errorString;
	bool m_autoFormatting;
	QString m_autoFormattingIndent;
	void *reserved;
};

class JSON {
public:
	static QString  stringify (const QVariant& v) {
		JsonWriter w;
		if (!w.stringify(v))
			qDebug() << w.errorString();
		return w.result();
	}
};

