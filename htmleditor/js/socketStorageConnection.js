(function (websocketInstance, storageInstance) {
	"use strict";
	
	$(websocketInstance).on('ondocument', function(d, doc) {
		if (doc.type_=="property") {
			if (doc.method_)
				storageInstance.modelChange(doc.method_, doc);
			else
				storageInstance.notification(doc);
		} else if (doc.type_=="ack") {
			if (api.isErrorAck(doc))
				console.warn("Error response:", doc)
				$.jGrowl("Fehlgeschlagen: "+doc.errormsg);
		} else if (doc.type_=="error") {
			console.warn("Server response:" + doc.msg)
		} else if (doc.method_=="changed")  {
			storageInstance.documentChanged(doc.document, {removed:false,reponseid:api.getReponseID(doc)});
		} else if (doc.method_=="removed")  {
			storageInstance.documentChanged(doc.document, {removed:true,reponseid:api.getReponseID(doc)});
		} else if (doc.method_=="batch")  {
			storageInstance.documentBatch(doc.documents);
		} else {
			console.warn("Response type unknown:", doc)
		}
	});
})( websocketInstance, storageInstance);
