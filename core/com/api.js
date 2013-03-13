exports.api = {
	needAck: function(remoteDoc) {
		return remoteDoc.type_!="ack" && remoteDoc.requestid_ && remoteDoc.requestid_!="";
	},
	isAck: function(remoteDoc, expect) {
		return remoteDoc.type_=="ack" && (expect ? remoteDoc.responseid_== expect : true);
	},
	isForStorage: function(remoteDoc) {
		return !remoteDoc.type_=="storage";
	},
	isServiceCall: function(remoteDoc) {
		return !remoteDoc.type_=="call";
	},
	// extract document from clientDoc
	getDataFromClientCall: function(clientDoc, sessionid) {
		var obj = clientDoc.doc;
		if (sessionid) obj.sessionid_ = sessionid;
		return obj;
	},
	generateAck: function(remoteDoc) {
		return {"type_":"ack","responseid_":remoteDoc.requestid_};
	},
	methodIdentify: function(requestid) {
		return {"method_":"identify","apiversion":10,"provides":"core","requestid_":requestid};
	},
	serviceAPI: {
		init: function() {
			return {"method_":"initialize"};
		},
		clear: function() {
			return {"method_":"clear"};
		},
		requestProperties: function() {
			return {"method_":"requestProperties"};
		}
	}
}
