/**
 * Storage Listener Management
 * If a connected client or internal objects want to be notified of
 * storage changes, they have to register to this storage lister via
 * addListener. The listener is updated by the storage object via the change/remove methods.
 */

var assert = require('assert');
var listener = {};

exports.addListener = function(obj, id) {
	assert(id, "storage.addListener: ID not set!");
	var d = listener[id];
	if (d)
		return;
	listener[id] = obj;
	
	// if obj sends document changes, we listen to them
	if (obj.on)
		obj.on("data", onListenerData);
	
// 	console.log('Add Storage listener:', id);
}

exports.removeListener = function(id) {
	assert(id, "storage.removeListener: ID not set!");

	var d = listener[id];
	if (!d)
		return;
	
	//console.log('Remove Storage listener:', id);
	if (d.removeListener)
		d.removeListener("data", onListenerData);
	delete listener[id];
}

/**
 * PRIVAT METHODS OR NON PUBLIC API *
 */

var storage = require('./storage.js');
var api = require('./com/api.js').api;

exports.change = function(doc, storageCode) {
	for (var i in listener) {
		console.log('Storage propagate:', i);
		listener[i].send(api.manipulatorAPI.methodDocumentUpdated(doc, storageCode));
	}
}

exports.remove = function(doc, storageCode) {
	for (var i in listener) {
		console.log('Storage propagate:', i);
		listener[i].send(api.manipulatorAPI.methodDocumentRemoved(doc, storageCode));
	}
}

function onListenerData(data, from) {
// 	console.log(data);
	if (!data.type_ || data.type_ != "storage")
		return;
	var responseID = api.getRequestID(data);
	
	function onStorageError(code, err) {
		console.warn("Storage error:", data);
		from.send(api.generateError(responseID, code, err));
	}
	
	if (api.manipulatorAPI.isFetchDocuments(data)) {
		storage.getDocuments(data.type, data.filter, function(err, items) {
			if (err) {
				return;
			}
			if (items) {
				for (var i=0;i<items.length;++i)
					delete items[i]._id; // remove mongodb id
				from.send({type_:"storage",method_:"batch",documents:items});
			}
		});
		return;
	} else if (api.manipulatorAPI.isDocumentUpdate(data)) {
		storage.update(api.manipulatorAPI.extractDocument(data), responseID, onStorageError);
		return;
	} else if (api.manipulatorAPI.isSceneItemDocumentInsert(data)) {
		storage.insertSceneItem(api.manipulatorAPI.extractDocument(data), responseID, onStorageError);
		return;
	} else if (api.manipulatorAPI.isDocumentRemove(data)) {
		storage.remove(api.manipulatorAPI.extractDocument(data), responseID, onStorageError);
		return;
	} else if (api.manipulatorAPI.isDocumentInsert(data)) {
		storage.insert(api.manipulatorAPI.extractDocument(data), responseID, onStorageError);
		return;
	} else {
		console.warn("Storage method not supported!", api.getMethod(data));
		from.send(api.generateError(responseID, "api.missing", "Storage method not supported!"));
	}
}
