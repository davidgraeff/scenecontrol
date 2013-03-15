(function (window) {
	"use strict";

	// Client API, common
	window.api = {
		needAck: function(remoteDoc) {
			return remoteDoc.type_!="ack" && remoteDoc.requestid_ && remoteDoc.requestid_!="";
		},
		isAck: function(remoteDoc, expect) {
			return remoteDoc.type_=="ack" && (expect ? remoteDoc.responseid_== expect : true);
		},
		getReponseID: function(remoteDoc) {
			return remoteDoc.responseid_;
		}, 
		generateAck: function(remoteDoc) {
			return {"type_":"ack","responseid_":remoteDoc.requestid_};
		}, 
		addRequestID: function(doc, requestid) {
			doc.requestid_ = requestid;
			return doc;
		},
		methodIdentify: function(requestid) {
			return {"method_":"identify","apiversion":10,"provides":["consumer","manipulator"],"requestid_":requestid, componentid_:"htmleditor"};
		}
	};
	// Client API, consumerAPI
	window.api.consumerAPI = {
		registerNotifier: function() {
			return({"method_":"registerNotifier"});
		},
		requestAllProperties: function() {
			return({"method_":"requestAllProperties"});
		},
		requestDocuments: function(type, filter) {
			return({method_:"fetchDocuments",type:type,filter:{}});
		}
	};
	// Client API, manipulatorAPI
	window.api.manipulatorAPI = {
		createScene: function(name) {
			return api.update({"type_": "scene","v":[],"categories": [],"enabled": true,"name": name}, true);
		},
		
		createSceneItem: function(scene_id, sceneItemDocument) {
			return api.update(sceneItemDocument, true);
		},
		
		removeSceneItem: function(scene_id, sceneItemDocument) {
			return api.remove(sceneDocument);
		},
		
		createConfig: function(instanceid, componentid) {
			return api.update({"componentid_":componentid, "type_": "configuration","instanceid_": instanceid}, true);
		},
		
		remove: function(sceneDocument) {
			return ({method_:"removeDocument","doc":sceneDocument});
		},
		
		update: function(sceneDocument, allowInsert) {
			//console.log("Change: ", sceneDocument)
			return {method_:"changeDocument",insert:(allowInsert?allowInsert:false), doc:sceneDocument};
		}
	};
})(window);