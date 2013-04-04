exports.api = {
	needAck: function(remoteDoc) {
		return remoteDoc.type_!="ack" && remoteDoc.requestid_ && remoteDoc.requestid_!="";
	},
	isAck: function(remoteDoc, expect) {
		return remoteDoc.type_=="ack" && (expect ? remoteDoc.responseid_== expect : true);
	},
	setAckRequired: function(remoteDoc, expect) {
		return remoteDoc.requestid_= expect;
	},
	generateAck: function(remoteDoc, reponse) {
		var obj = {type_:"ack", responseid_:remoteDoc.requestid_};
		if (reponse) obj.response_ = reponse;
		return obj;
	},
	methodIdentify: function(requestid) {
		return {method_:"identify",apiversion:10,provides:"core",requestid_:requestid};
	},
	isExecuteCall: function(remoteDoc) {
		return remoteDoc.method_=="execute";
	},
	getMethod: function(remoteDoc) {
		return remoteDoc.method_;
	},
	getRequestID: function(remoteDoc) {
		return remoteDoc.requestid_;
	},
	generateError: function(requestid, errorcodeid, errormsg) {
		return {type_:"ack", method_:"error", requestid_:requestid, errorcode: exports.errorcodes.indexOf(errorcodeid), errormsg:errormsg};
	},
	uidEqual: function(doc1, doc2) {
		return doc1.type_==doc2.type_ && doc1.id_==doc2.id_;
	}
};

exports.errorcodes = ["api.missing", "storage.insert", "storage.update", "storage.remove"];

exports.api.manipulatorAPI = {
	isFetchDocuments: function(remoteDoc) {
		return remoteDoc.method_=="fetch";
	},
	isDocumentInsert: function(remoteDoc) {
		return remoteDoc.method_=="insert";
	},
	isSceneItemDocumentInsert: function(remoteDoc) {
		return remoteDoc.method_=="insertSceneItem";
	},
	isDocumentUpdate: function(remoteDoc) {
		return remoteDoc.method_=="update";
	},
	isDocumentRemove: function(remoteDoc) {
		return remoteDoc.method_=="remove";
	},
	methodDocumentUpdated: function(storageDoc, storageCode) {
		delete storageDoc._id; // remove mongodb id
		return {method_:"changed", responseid_:storageCode, document:storageDoc};
	},
	methodDocumentRemoved: function(storageDoc, storageCode) {
		delete storageDoc._id; // remove mongodb id
		return {method_:"removed", responseid_:storageCode, document:storageDoc};
	},
	extractDocument: function(remoteDoc) {
		return remoteDoc.document;
	}
};

exports.api.consumerAPI = {
	isRequestAllProperties: function(remoteDoc) {
		return remoteDoc.method_=="requestAllProperties";
	}
};

exports.api.serviceAPI = {
	modelReset: function(id, key) {
		return {type_:"model",method_:"reset",id_:id, key_:key};
	},
	init: function() {
		return {"method_":"initialize"};
	},
	clear: function() {
		return {"method_":"clear"};
	},
	/**
	 * Abort execution of all conditions, actions currently running 
	 * belonging to the scene given by the sceneid.
	 */
	abortExecution: function(sceneid) {
		return {"method_":"abort",sceneid_:sceneid};
	},
	requestProperties: function() {
		return {"method_":"requestProperties"};
	},
	isServiceCall: function(remoteDoc) {
		return remoteDoc.method_=="call";
	},
	isPropertyChange: function(remoteDoc) {
		return remoteDoc.type_=="property";
	},
	isTriggeredEvent: function(remoteDoc) {
		return remoteDoc.method_=="eventTriggered";
	},
	// extract document from clientDoc
	getDataFromClientCall: function(clientDoc, sessionid) {
		var obj = clientDoc.doc;
		if (sessionid && sessionid!=-1) obj.sessionid_ = sessionid;
		return obj;
	}
};