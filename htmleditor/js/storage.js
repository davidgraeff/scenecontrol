/**
 * Storage Module - Store all scenes, scene items and configuration from the SceneServer.
 * Available as window.storageInstance.
 */
(function (window) {
	"use strict";
	function Storage() {
		var that = this;
		that.notifier = false;
		that.data = {};
// 		that.events = {};
// 		that.conditions = {};
// 		that.actions = {};
		that.models = {};
		that.plugins = []; // List of plugins

		this.getComponentName = function(componentid) {
			var descs = this.data["description"];
			if (descs) {
				for (var i in descs) {
					if (descs[i].componentid_ == componentid)
						return descs[i].name;
				}
			}
			return componentid;
		}
		
		this.configurationsForPlugin = function(componentid) {
			var configurations = this.data["configuration"];
			var result = []
			for (var index in configurations)
				if (configurations[index].componentid_ == componentid)
					result.push(configurations[index]);
			return result;
		}
		
		this.configInstanceIDUsed = function(componentid, instanceid) {
			var configurations = this.data["configuration"];
			for (var index in configurations)
				if (configurations[index].componentid_ == componentid &&
					configurations[index].instanceid_ == instanceid)
					return true;
			return false;
		}
		
		this.documentsForScene = function(sceneid) {
			var result = []
			for (var typeOfSceneItem in ["event","condition","action"])
				for (var index in this.data[typeOfSceneItem]) {
					var d = this.data[typeOfSceneItem][index];
					if (d.sceneid_ == sceneid)
					result.push(d);
				}
			return result;
		}
		
		this.modelItems = function(componentid, instanceid, modelid) {
// 			return that.models[componentid+instanceid+modelid];
			return that.models[modelid];
		}
		
		this.componentIDsFromConfigurationsByType = function(type) {
			var schemas = this.data["schema"];
			var hasSchema = {};
			for (var index in schemas) {
				if (schemas[index].targettype == type) {
					var cid = schemas[index].componentid_;
					if (hasSchema[cid])
						continue;
					hasSchema[cid] = 1;
				}
			}
			// take all configurations where the plugins have schemas for the given type
			var configurations = this.data["configuration"];
			var result = [];
			for (var index in configurations) {
				if (hasSchema[configurations[index].componentid_]) {
					result.push(
						{componentid_:configurations[index].componentid_,
						instanceid_:configurations[index].instanceid_}
					);
				}
			}
			return result;
		}
		
		this.schemaForPlugin = function(componentid, type) {
			var result = [];
			this.forEveryDocument("schema", function(item) {
				if (item.componentid_ == componentid && item.targettype == type)
					result.push(item);
			});
			return result;
		}
		
		this.schemaForDocument = function(doc) {
			return this.firstDocument("schema", function(item) {
				return (item.componentid_ == doc.componentid_ && item.method_ == doc.method_ && item.targettype == doc.type_)
			});
		}
		
		this.getID = function(sceneDocument) {
				return sceneDocument.id_;
		}
		
		this.getDocument = function(type, id){
			return that.data[type][id];
		}
		
		this.forEveryDocument = function(type, callback){
			var items = that.data[type];
			for (var index in items)
				callback(items[index]);
		}
		
		this.firstDocument = function(type, callback){
			var items = that.data[type];
			for (var index in items) {
				if (callback(items[index]))
					return items[index];
			}
			return null;
		}

		this.documentChanged = function(doc, flags) {
			if (!doc.type_) {
				console.warn("Document not accepted!", doc);
				return;
			}
// 			console.log("docchanged", doc, flags);
			
			if (flags.removed)
				delete that.data[doc.type_][this.getID(doc)];
			else {
				if (!that.data[doc.type_])
					that.data[doc.type_] = {};
				that.data[doc.type_][this.getID(doc)] = doc;
			}
			
			that.notifyDocumentChange(doc, flags);
		};
		
		this.notifyDocumentChange = function(doc, flags) {
			flags.doc = doc;
			//console.log("on"+doc.type_);
			$(that).trigger("on"+doc.type_, flags);
		};
		
		this.modelChange = function(action, doc) {
			//var modelid = doc.componentid_+doc.instanceid_+doc.id_;
			var modelid = doc.id_;
			if (action=="reset") {
				that.models[modelid] = {"key":doc.key_,"data":{}};
				$(that).trigger("model.reset", modelid);
			} else if (action=="change") {
				var model = that.models[modelid];
				if (model == null || model["key"] == null)
					return;
				var key = doc[ model["key"] ];
				if (key == null)
					return;
				delete doc.method_;
				delete doc.type_;
				model.data[key] = doc;
				$(that).trigger("model.change", modelid);
			} else if (action=="remove") {
				var model = that.models[modelid];
				if (model == null || model["key"] == null)
					return;
				var key = doc[ model["key"] ];
				if (key == null)
					return;
				delete doc.method_;
				delete doc.type_;
				delete model.data[key];
				$(that).trigger("model.remove", modelid);
			}
		};
		
		this.documentBatch = function(docs) {
			for (var i = 0; i < docs.length; i++) {
				that.documentChanged(docs[i], {batch:true});
			}
		};

		this.notification = function(doc) {
			if (doc.method_=="pluginlist")  {
				that.plugins = doc.plugins;
				$(that).trigger("onplugins");
			} else {
				$(that).trigger("onnotification", doc);
				console.log("Notification", doc);
			}
		};
		
		this.clear = function() {
			that.data = {};
			that.models = {};
			that.plugins = [];
		}
		
		this.escapeInputForJson = function (text)
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
	}

	window.storageInstance = new Storage();
})(window);
