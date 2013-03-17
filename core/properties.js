var propertyListener = require('./properties.listener.js');

exports.properties = {};
exports.propertyModels = {};

exports.change = function(doc, from) {
	var data;
	var id = from.componentid_+""+from.instanceid_;
	
	if (!exports.properties[id])
		exports.properties[id] = {normal:{},model:{}};
	
	if (doc.method_) { // model property
		if (!exports.properties[id].model[doc.id_])
			exports.properties[id].model[doc.id_] = {};
		
		if (doc.method_=="change") {
			var info = exports.properties[id].model[doc.id_].info;
			if (!info)
				return;
			
			exports.properties[id].model[doc.id_].data[doc[info.key_]] = doc;
		}
		else if (doc.method_=="remove") {
			var info = exports.properties[id].model[doc.id_].info;
			if (!info)
				return;
			delete exports.properties[id].model[doc.id_].data[doc[info.key_]];
		}
		else if (doc.method_=="reset") {
			exports.properties[id].model[doc.id_] = {info:doc,data:{}}
		} else {
			console.warn("Properties: Model method not known!", doc.method_);
		}
	} else // normal property
		exports.properties[id].normal[doc.id_] = doc;
	
	propertyListener.change(doc);
}

exports.apply = function(propName) {
	for (var i in exports.properties) {
		var serviceProperties = exports.properties[i];
		
		if (serviceProperties.normal) {
			for (var j in serviceProperties.normal) {
				var prop = serviceProperties.normal[j];
				if (prop.id_ == propName)
					return prop;
			}
		}
	}
	return null;
}


exports.requestAllProperties = function(callback) {
	var propertieslist = [];
	for (var i in exports.properties) {
		var serviceProperties = exports.properties[i];
		
		if (serviceProperties.normal) {
			for (var j in serviceProperties.normal) {
				propertieslist.push(serviceProperties.normal[j]);
			}
		}
		
		
		if (serviceProperties.model) {
			for (var i in serviceProperties.model) {
				var model = serviceProperties.model[i];
				propertieslist.push(model.info);
				for (var j in model.data) {
					propertieslist.push(model.data[j]);
				}
			}
		}
	}

	callback(propertieslist);
}
