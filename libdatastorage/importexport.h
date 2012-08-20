/*
    RoomControlServer. Home automation for controlling sockets, leds and music.
    Copyright (C) 2012  David Gräff

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Purpose: Datastorage import from json documents and export to json documents
*/

#pragma once
#include <QVariantMap>
#include <QString>
#include <QFileInfo>
#include <shared/jsondocuments/scenedocument.h>

namespace Datastorage {
	/* Derive from this interface to use your own verifier for importing and exporting json documents */
	class VerifyInterface {
		public:
		virtual bool isValid(SceneDocument& data, const QString& filename) = 0;
	};
	/* Use this verifier class for importing plugin json documents. It will add some information before the document
	 * is send to the datastorage
	 */
	class VerifyPluginDocument : public VerifyInterface {
		private:
			QString m_pluginid;
		public:
		VerifyPluginDocument(const QString& pluginid) : m_pluginid(pluginid) {}
		virtual bool isValid(SceneDocument& data, const QString& filename) {
			data.setPluginid(m_pluginid);
			if (!data.id().isEmpty())
            	data.setid((QString)(QFileInfo(filename).completeBaseName() + m_pluginid));
			return true;
		}
	};

/**
 * Export all json documents to the given path (synchronous)
 */
void exportAsJSON(const DataStorage& ds, const QString& exportpath);
/**
 * Import all json documents from the given path recursively (synchronous)
 */
void importFromJSON(DataStorage& ds, const QString& path, bool overwriteExisting = false, Datastorage::VerifyInterface* verify = 0);

}
