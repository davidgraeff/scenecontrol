(function (websocketInstance, storageInstance) {
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
		$.jGrowl("Keine Verbindung zum Server!");
		$.mobile.loading( 'hide' );
	});

	$(websocketInstance).on('onidentified', function() {
		websocketInstance.write(api.manipulatorAPI.fetchDocuments("scene",{}));
		websocketInstance.write(api.manipulatorAPI.fetchDocuments("event",{}));
		websocketInstance.write(api.manipulatorAPI.fetchDocuments("condition",{}));
		websocketInstance.write(api.manipulatorAPI.fetchDocuments("action",{}));
		websocketInstance.write(api.manipulatorAPI.fetchDocuments("schema",{}));
		websocketInstance.write(api.manipulatorAPI.fetchDocuments("description",{}));
		websocketInstance.write(api.manipulatorAPI.fetchDocuments("configuration",{}));
		
		websocketInstance.write(api.consumerAPI.requestAllProperties());
		window.loadPage('scenelist');
		$("header nav").removeClass("hidden");
		$.mobile.loading( 'hide' );
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
	
})(websocketInstance, storageInstance);

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
