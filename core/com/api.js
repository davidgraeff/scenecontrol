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
		var obj = {"type_":"ack","responseid_":remoteDoc.requestid_};
		if (reponse) obj.response_ = reponse;
		return obj;
	},
	methodIdentify: function(requestid) {
		return {"method_":"identify","apiversion":10,"provides":"core","requestid_":requestid};
	}
};

exports.api.manipulatorAPI = {
	isDocumentUpdate: function(remoteDoc) {
		return remoteDoc.method_=="storage";
	},
	isDocumentRemove: function(remoteDoc) {
		return remoteDoc.method_=="storage";
	}
};

exports.api.consumerAPI = {
	isFetchDocuments: function(remoteDoc) {
		return remoteDoc.method_=="fetchDocuments";
	},
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
		if (sessionid) obj.sessionid_ = sessionid;
		return obj;
	}
};