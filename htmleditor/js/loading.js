(function (document, websocketInstance, storageInstance) {
	"use strict";
	
	$(function() {
		$("#servernameandport").val(websocketInstance.defaultHostAndPort());
		
		$('#btnConnectToServer').on('click', function() {
			$.mobile.loading( 'show', { theme: "b", text: "Lade Dokumente", textonly: false });
			websocketInstance.setHostAndPort($("#servernameandport").val(), false);
			websocketInstance.reconnect();
		});
	});

	$(websocketInstance).on('onclose', function() {
		window.location = window.location.href.replace( /#.*/, "");
	});
	
	$(websocketInstance).on('onerror', function() {
		$.jGrowl("Keine Verbindung zum Server<br/>Läuft der Websocketproxy?");
		$.mobile.loading( 'hide' );
	});
	
	$(websocketInstance).on('onopen', function() {
		websocketInstance.requestAllDocuments();
		websocketInstance.registerNotifier();
		websocketInstance.requestAllProperties();
	});

	$(storageInstance).on('onloadcomplete', function() {
		window.loadPage('scenelist');
		$.mobile.loading( 'hide' );
	});
	
	window.prepareLinks = function() {
		
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
})(jQuery);
