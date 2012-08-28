function Storage() {
	var that = this;
	that.notifier = false;
	that.scenes = {};
	that.schemas = {};
	that.events = {};
	that.conditions = {};
	that.actions = {};

	this.documentChanged = function(doc, removed) {
		var id_ = doc.id_;
		if (doc.type_=="scene") {
			that.scenes[id_] = doc;
			$(that).trigger("onscene", doc, removed);
		} else if (doc.type_=="schemas") {
			that.schemas[id_] = doc;
			$(that).trigger("onschemas", doc, removed);
		} else if (doc.type_=="event") {
			that.events[id_] = doc;
			$(that).trigger("onevent", doc, removed);
		} else if (doc.type_=="condition") {
			that.conditions[id_] = doc;
			$(that).trigger("oncondition", doc, removed);
		} else if (doc.type_=="action") {
			that.actions[id_] = doc;
			$(that).trigger("onaction", doc, removed);
		}
	};
	
	$(websocketInstance).bind('ondocument', function(d, doc) {
		if (doc.type_!="notification")
			return;
		if (doc.id_=="alldocuments") {
			for (var i = 0; i < doc.documents.length; i++) {
				that.documentChanged(doc.documents[i], false);
			}
		} else if (doc.id_=="documentChanged")  {
			that.documentChanged(doc.document, false);
		} else if (doc.id_=="documentRemoved")  {
			that.documentChanged(doc.document, true);
		}
	});
	
	$(websocketInstance).bind('onopen', function() {
		websocketInstance.requestAllDocuments();
		websocketInstance.registerNotifier();
	});
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
	return text;
};

storageInstance = new Storage();
