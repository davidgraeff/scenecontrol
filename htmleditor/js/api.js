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
 		isErrorAck: function(remoteDoc) {
			return remoteDoc.type_=="ack" && remoteDoc.method_ && remoteDoc.method_=="error";
		},
		getReponseID: function(remoteDoc) {
			return remoteDoc.responseid_;
		}, 
		generateAck: function(remoteDoc) {
			return {type_:"ack",responseid_:remoteDoc.requestid_};
		}, 
		addRequestID: function(doc, requestid) {
			doc.requestid_ = requestid;
			return doc;
		},
		methodIdentify: function(requestid) {
			return {"method_":"identify","apiversion":10,"provides":["consumer","manipulator"],"requestid_":requestid, componentid_:"htmleditor"};
		},
		uidFromDocument: function(doc) {
			return {type_:doc.type_,id_:doc.id_};
		},
		uidEqual: function(doc1, doc2) {
			return doc1.type_==doc2.type_ && doc1.id_==doc2.id_;
		}
	};
	// Client API, consumerAPI
	window.api.consumerAPI = {
		registerNotifier: function() {
			return {type_:"consumer", "method_":"registerNotifier"};
		},
		requestAllProperties: function() {
			return {type_:"consumer", "method_":"requestAllProperties"};
		}
	};
	// Client API, manipulatorAPI
	window.api.manipulatorAPI = {
		createScene: function(name) {
			return api.manipulatorAPI.insert({"type_": "scene","v":[],"categories": [],"enabled": true,"name": name});
		},
		
		createSceneItem: function(sceneItemDocument) {
			return {type_:"storage", method_:"insertSceneItem", document:sceneItemDocument};
		},
				
		updateSceneItem: function(sceneItemDocument) {
			return api.manipulatorAPI.update(sceneItemDocument);
		},
		
		removeSceneItem: function(sceneItemDocument) {
			return api.manipulatorAPI.remove(sceneItemDocument);
		},
		
		createConfig: function(instanceid, componentid) {
			return api.manipulatorAPI.insert({"componentid_":componentid, "type_": "configuration","instanceid_": instanceid});
		},
		
		remove: function(sceneDocument) {
			return {type_:"storage", method_:"remove",document:sceneDocument};
		},
		
		update: function(sceneDocument) {
			return {type_:"storage", method_:"update", document:sceneDocument};
		},
 
		insert: function(sceneDocument) {
			return {type_:"storage", method_:"insert", document:sceneDocument};
		},
 
		fetchDocuments: function(type, filter) {
			return {type_:"storage", method_:"fetch",type:type,filter:filter};
		}
	};
})(window);