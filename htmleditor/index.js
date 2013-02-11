(function (document, websocketInstance, storageInstance) {
	"use strict";
	
	$(document).on('pageinit', function() {
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
		$.mobile.changePage('editor.html', {transition: 'slide'});
		$.mobile.loading( 'hide' );
	});
})(document, websocketInstance, storageInstance);