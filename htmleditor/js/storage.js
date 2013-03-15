/**
 * Storage Module - Store all scenes, scene items and configuration from the SceneServer.
 * Available as window.storageInstance.
 */
(function (window) {
	"use strict";
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

		this.configurationsForPlugin = function(pluginid) {
			var result = []
			for (var index in this.configurations)
				if (this.configurations[index].componentid_ == pluginid)
					result.push(this.configurations[index]);
			return result;
		}
		
		this.configInstanceIDUsed = function(pluginid, instanceid) {
			for (var index in this.configurations)
				if (this.configurations[index].componentid_ == pluginid &&
					this.configurations[index].instanceid_ == instanceid)
					return true;
			return false;
		}
		
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
		
		// Find the document referenced by docID, docType, docComponentID and add basic information to baseDoc to be a valid SceneDocument
	// 	this.addEssentialDocumentData = function(docID, docType, docComponentID, baseDoc) {
	// 		var o = {};
	// 		var id_ = docComponentID+docID;
	// 		if (docType=="scene") {
	// 			o = that.scenes[id_];
	// 		} else if (docType=="event") {
	// 			o = that.events[id_];
	// 		} else if (docType=="condition") {
	// 			o = that.conditions[id_];
	// 		} else if (docType=="action") {
	// 			o = that.actions[id_];
	// 		} else if (docType=="configuration") {
	// 			o = that.configurations[id_];
	// 		} else
	// 			return null;
	// 		
	// 		if (!o)
	// 			return null;
	// 		
	// 		baseDoc.id_ = o.id_;
	// 		baseDoc.type_ = o.type_;
	// 		baseDoc.componentid_ = o.componentid_;
	// 		if (o.instanceid_) baseDoc.instanceid_ = o.instanceid_;
	// 		if (o.method_) baseDoc.method_ = o.method_;
	// 		if (o.sceneid_) baseDoc.sceneid_ = o.sceneid_;
	// 		return baseDoc;
	// 	}
		
		this.modelItems = function(componentid, instanceid, modelid) {
// 			return that.models[componentid+instanceid+modelid];
			return that.models[modelid];
		}
		
		this.componentIDsFromConfigurationsByType = function(type) {
			var hasSchema = {};
			for (var index in this.schemas) {
				if (this.schemas[index].targettype == type) {
					var cid = this.schemas[index].componentid_;
					if (hasSchema[cid])
						continue;
					hasSchema[cid] = 1;
				}
			}
			// take all configurations where the plugins have schemas for the given type
			var result = [];
			for (var index in this.configurations) {
				if (hasSchema[this.configurations[index].componentid_]) {
					result.push(
						{componentid_:this.configurations[index].componentid_,
						instanceid_:this.configurations[index].instanceid_}
					);
				}
			}
			return result;
		}
		
		this.schemaForPlugin = function(pluginid, type) {
			var result = [];
			for (var index in this.schemas)
				if (this.schemas[index].componentid_ == pluginid && this.schemas[index].targettype == type)
					result.push(this.schemas[index]);
			return result;
		}
		
		this.schemaForDocument = function(doc) {
			for (var index in this.schemas)
				if (this.schemas[index].componentid_ == doc.componentid_ && this.schemas[index].method_ == doc.method_ && this.schemas[index].targettype == doc.type_)
					return this.schemas[index];
			return null;
		}
		
		/**
		* Return a unique id for documents without an id that is unique accross all components.
		* Use this this for schemas and configurations.
		*/
		this.uniqueComponentID = function(SceneDocument) {
			return SceneDocument.componentid_+SceneDocument.id_;
		}
		
		this.getDocument = function(type, id){
			if (type=="event") {
				return that.events[id];
			} else if (type=="condition") {
				return that.conditions[id];
			} else if (type=="action") {
				return that.actions[id];
			}
			return null;
		}

		this.documentChanged = function(doc, flags) {
			var removed = flags.removed;
			if (doc.type_=="scene") {
				if (removed)
					delete that.scenes[doc.id_];
				else
					that.scenes[doc.id_] = doc;
			} else if (doc.type_=="schema") {
				if (removed)
					delete that.schemas[this.uniqueComponentID(doc)];
				else
					that.schemas[this.uniqueComponentID(doc)] = doc;
			} else if (doc.type_=="event") {
				if (removed)
					delete that.events[doc.id_];
				else
					that.events[doc.id_] = doc;
			} else if (doc.type_=="condition") {
				if (removed)
					delete that.conditions[doc.id_];
				else
					that.conditions[doc.id_] = doc;
			} else if (doc.type_=="action") {
				if (removed)
					delete that.actions[doc.id_];
				else
					that.actions[doc.id_] = doc;
			} else if (doc.type_=="configuration") {
				if (removed)
					delete that.configurations[this.uniqueComponentID(doc)];
				else
					that.configurations[this.uniqueComponentID(doc)] = doc;
			}
			
			if (doc.type_)
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
			that.scenes = {};
			that.schemas = {};
			that.events = {};
			that.conditions = {};
			that.actions = {};
			that.configurations = {};
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
