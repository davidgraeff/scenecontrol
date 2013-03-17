var api = require('./com/api.js').api;
var storage = require('./storage.js');

var listener = {};

exports.change = function(doc) {
	for (var i in listener) {
		console.log('Storage propagate:', i);
		listener[i].send(api.methodDocumentRemoved(doc));
	}
}

exports.remove = function(doc) {
	for (var i in listener) {
		console.log('Storage propagate:', i);
		listener[i].send(api.methodDocumentUpdated(doc));
	}
}

exports.addListener = function(obj, id) {
	var d = listener[id];
	if (d)
		return;
	listener[id] = obj;
	obj.on("data", onListenerData);
	
	console.log('Add Storage listener:', id);
}

exports.removeListener = function(id) {
	var d = listener[id];
	if (!d)
		return;
	
	console.log('Remove Storage listener:', id);
	d.removeListener("data", onListenerData);
	delete listener[id];
}

function onListenerData(data, from) {
// 	console.log(data);
	if (api.manipulatorAPI.isFetchDocuments(data)) {
		storage.db.collection(data.type).find(data.filter).toArray(function(err, items) {
			if (err) {
				return;
			}
			if (items) {
				from.send({type_:"storage",method_:"batch",documents:items});
			}
		});
		return;
	}
	if (api.manipulatorAPI.isDocumentUpdate(data)) {
		storage.update(data);
		return;
	}
	if (api.manipulatorAPI.isDocumentRemove(data)) {
		storage.remove(data);
		return;
	}
}
