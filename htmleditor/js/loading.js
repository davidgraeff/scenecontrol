(function (document, websocketInstance, storageInstance) {
	"use strict";
	
	$.ajaxSetup ({
		// Disable caching of AJAX responses
		cache: false
	});
		
	$(function() {
		var rememeber = localStorage.getItem("storehostAndPort");
		if (remember)
			$("#servernameandport").val(localStorage.getItem("hostAndPort"));
		else
			$("#servernameandport").val(websocketInstance.defaultHostAndPort());
		
		$("header nav").addClass("hidden");
		
		$('#btnConnectToServer').on('click', function() {
			$.mobile.loading( 'show', { theme: "b", text: "Lade Dokumente", textonly: false });
			var rememeber = $("#remember").is(':checked');
			if (rememeber==true) {
				localStorage.setItem("hostAndPort", $("#servernameandport").val());
			}
			localStorage.setItem("storehostAndPort", remember);
			websocketInstance.setHostAndPort($("#servernameandport").val(), false);
			websocketInstance.reconnect();
		});
	});

	$(websocketInstance).on('onclose', function() {
		window.location = window.location.href.replace( /#.*/, "");
		storageInstance.clear();
	});
	
	$(websocketInstance).on('onerror', function() {
		$.jGrowl("Keine Verbindung zum Server<br/>Läuft der Websocketproxy?");
		$.mobile.loading( 'hide' );
	});

	$(websocketInstance).on('onidentified', function() {
		websocketInstance.write(api.consumerAPI.requestDocuments("scene",{}));
		websocketInstance.write(api.consumerAPI.requestDocuments("event",{}));
		websocketInstance.write(api.consumerAPI.requestDocuments("condition",{}));
		websocketInstance.write(api.consumerAPI.requestDocuments("action",{}));
		websocketInstance.write(api.consumerAPI.requestDocuments("schema",{}));
		websocketInstance.write(api.consumerAPI.requestDocuments("description",{}));
		websocketInstance.write(api.consumerAPI.requestDocuments("configuration",{}));
		
		websocketInstance.write(api.consumerAPI.requestAllProperties());
		window.loadPage('scenelist');
		$("header nav").removeClass("hidden");
		$.mobile.loading( 'hide' );
	});
	
	$(websocketInstance).on('ondocument', function(d, doc) {
		if (doc.type_=="property") {
			if (doc.method_)
				storageInstance.modelChange(doc.method_, doc);
			else
				storageInstance.notification(doc);
		} else if (doc.type_=="ack") {
// 			console.warn("Server response:" + doc.msg)
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
	
	window.prepareLinks = function() {
		$('.autolink').on('click', function() {
			var ref = this.href.split("#")[1];
			if (ref)
				loadPage(ref);
		});
	}
	
	window.loadPage = function(pageid) {
		$('#maincontent').load('pages/'+pageid+'.html');
		prepareLinks();
	}
	
	// dummies
	$.mobile = {loading:function(){},hide:function(){},silentScroll:function(){}}
	
})(document, websocketInstance, storageInstance);

// jquery plugin for handlebars
(function($) {
	var compiled = {};
	$.fn.handlebarsAfter = function(template, data) {
		var ctx;
		if (compiled[template]!=null)
			ctx = compiled[template];
		else {
			ctx = Handlebars.compile($(template).html());
			compiled[template] = ctx;
		}
		
		this.after(ctx(data));
	};
	$.fn.handlebarsAppend = function(template, data) {
		var ctx;
		if (compiled[template]!=null)
			ctx = compiled[template];
		else {
			ctx = Handlebars.compile($(template).html());
			compiled[template] = ctx;
		}
		
		this.append(ctx(data));
	};
})(jQuery);
