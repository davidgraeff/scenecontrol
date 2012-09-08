function Storage() {
	var that = this;
	that.notifier = false;
	that.scenes = {};
	that.schemas = {};
	that.events = {};
	that.conditions = {};
	that.actions = {};
	that.configurations = {};
	that.models = {};
	that.plugins = []; // List of plugins

	this.documentsForScene = function(sceneid) {
		var result = []
		for (var index in this.events)
			if (this.events[index].sceneid_ == sceneid)
				result.push(this.events[index]);
		for (var index in this.conditions)
			if (this.conditions[index].sceneid_ == sceneid)
				result.push(this.conditions[index]);
		for (var index in this.actions)
			if (this.actions[index].sceneid_ == sceneid)
				result.push(this.actions[index]);
		return result;
	}
	
	this.modelItems = function(componentid, instanceid, modelid) {
		return that.models[componentid+instanceid+modelid];
	}
	
	this.schemaForDocument = function(doc) {
		for (var index in this.schemas)
			if (this.schemas[index].componentid_ == doc.componentid_ && this.schemas[index].method_ == doc.method_ && this.schemas[index].targettype == doc.type_)
				return this.schemas[index];
		return null;
	}
	
	this.documentChanged = function(doc, removed) {
		var id_ = doc.id_;
		if (doc.type_=="scene") {
			that.scenes[id_] = doc;
		} else if (doc.type_=="schema") {
			that.schemas[id_] = doc;
		} else if (doc.type_=="event") {
			that.events[id_] = doc;
		} else if (doc.type_=="condition") {
			that.conditions[id_] = doc;
		} else if (doc.type_=="action") {
			that.actions[id_] = doc;
		} else if (doc.type_=="configuration") {
			that.configurations[doc.componentid_+doc.instanceid_] = doc;
		}
	};
	
	this.notifyDocumentChange = function(doc, removed) {
		var id_ = doc.id_;
		if (doc.type_=="scene") {
			doc.temp_ = {"counter":that.documentsForScene(id_).length}
			$(that).trigger("onscene", doc, removed);
		} else if (doc.type_=="schemas") {
			$(that).trigger("onschemas", doc, removed);
		} else if (doc.type_=="event") {
			$(that).trigger("onevent", doc, removed);
		} else if (doc.type_=="condition") {
			$(that).trigger("oncondition", doc, removed);
		} else if (doc.type_=="action") {
			$(that).trigger("onaction", doc, removed);
		} else if (doc.type_=="configuration") {
			$(that).trigger("onconfiguration", doc, removed);
		}
	};
	
	$(websocketInstance).bind('ondocument', function(d, doc) {
		if (doc.type_=="model.reset") {
			that.models[doc.componentid_+doc.instanceid_+doc.id_] = {"key":doc.configkey_,"data":{}};
			$(that).trigger("model.reset", doc.componentid_+doc.instanceid_+doc.id_);
		} else
		if (doc.type_=="model.change") {
			var model = that.models[doc.componentid_+doc.instanceid_+doc.id_];
			if (model == null || model["key"] == null)
				return;
			var key = doc[ model["key"] ];
			if (key == null)
				return;
			delete doc.method_;
			delete doc.type_;
			model.data[key] = doc;
			$(that).trigger("model.change", doc.componentid_+doc.instanceid_+doc.id_, doc);
		} else
		if (doc.type_=="model.remove") {
			var model = that.models[doc.componentid_+doc.instanceid_+doc.id_];
			if (model == null || model["key"] == null)
				return;
			var key = doc[ model["key"] ];
			if (key == null)
				return;
			delete doc.method_;
			delete doc.type_;
			delete model.data[key];
			$(that).trigger("model.remove", doc.componentid_+doc.instanceid_+doc.id_, doc);
			
		} else
		if (doc.type_=="notification") {
			if (doc.id_=="alldocuments") {
				for (var i = 0; i < doc.documents.length; i++) {
					that.documentChanged(doc.documents[i], false);
				}
				// notify about scenes
				for (var index in that.scenes) {
					that.notifyDocumentChange(that.scenes[index], false);
				}
				
				$(that).trigger("onloadcomplete");
			} else if (doc.id_=="documentChanged")  {
				that.documentChanged(doc.document, false);
				that.notifyDocumentChange(doc.document, false);
				// update scene
				if (doc.document.type_=="action"||doc.document.type_=="event"||doc.document.type_=="condition")
					that.notifyDocumentChange(doc.document.sceneid_, false);
			} else if (doc.id_=="documentRemoved")  {
				that.documentChanged(doc.document, true);
				that.notifyDocumentChange(doc.document, true);
				// update scene
				if (doc.document.type_=="action"||doc.document.type_=="event"||doc.document.type_=="condition")
					that.notifyDocumentChange(doc.document.sceneid_, false);
			} else if (doc.id_=="registerNotifier")  {
				console.log("Document notifier registered:", doc.notifierstate);
			} else if (doc.id_=="plugins" && doc.componentid_=="PluginController")  {
				that.plugins = doc.plugins;
				$(that).trigger("onplugins");
			} else {
				console.log("Notification", doc);
			}
		}
	});
	
	$(websocketInstance).bind('onopen', function() {
		websocketInstance.requestAllDocuments();
		websocketInstance.registerNotifier();
		websocketInstance.requestAllProperties();
	});
	
	$(websocketInstance).bind('onclose', function() {
		that.clear();
	});
	
	this.clear = function() {
		that.scenes = {};
		that.schemas = {};
		that.events = {};
		that.conditions = {};
		that.actions = {};
		that.configurations = {};
		that.models = {};
		that.plugins = [];
	}
}

function escapeInputForJson(text)
{
      if (text == null) return null;
	var _escapeable = /["\\\x00-\x1f\x7f-\x9f]/g;

	var _meta = {
		'\b': '\\b',
		'\t': '\\t',
		'\n': '\\n',
		'\f': '\\f',
		'\r': '\\r',
		'"' : '\\"',
		'\\': '\\\\'
	};

	if (text.match(_escapeable))
	{
		return text.replace(_escapeable, function (a) 
		{
			var c = _meta[a];
			if (typeof c === 'string') return c;
			c = a.charCodeAt();
			return '\\u00' + Math.floor(c / 16).toString(16) + (c % 16).toString(16);
		});
	}
	return  $.trim(text);
};

storageInstance = new Storage();
