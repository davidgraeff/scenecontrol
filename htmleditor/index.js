(function (document, websocketInstance, storageInstance) {
	"use strict";
	
	$(document).on('pageinit', function() {
		$("#servernameandport").val(websocketInstance.defaultHostAndPort());
		
		$('#btnConnectToServer').on('click', function() {
			$.mobile.loading( 'show', { theme: "b", text: "Lade Dokumente", textonly: false });
			websocketInstance.setHostAndPort($("#servernameandport").val());
			websocketInstance.reconnect();
		});
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