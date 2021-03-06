var api = require('./com/api.js').api;
var properties = require('./properties.js');

var listener = {};

exports.change = function(doc) {
	for (var i in listener) {
// 		console.log('Property propagate:', i);
		listener[i].send(doc);
	}
}

exports.addListener = function(obj, id) {
	var d = listener[id];
	if (d)
		return;
	listener[id] = obj;
	obj.on("data", onListenerData);
	
	console.log('Add Property listener:', id);
}

exports.removeListener = function(id) {
	var d = listener[id];
	if (!d)
		return;
	
	console.log('Remove Property listener:', id);
	d.removeListener("data", onListenerData);
	delete listener[id];
}

function onListenerData(data, from) {
	if (!data.type_ || data.type_ != "consumer")
		return;
	
	var responseID = api.getRequestID(data);
	
	if (api.consumerAPI.isRequestAllProperties(data)) {
		properties.requestAllProperties(function(items) {
			items.forEach(function(item) {
				from.send(item);
			});
		});
		return;
	} else {
		console.warn("Consumer method not supported!", api.getMethod(data));
		from.send(api.generateError(responseID, "api.missing",  "Consumer method not supported!"));
	}
}
