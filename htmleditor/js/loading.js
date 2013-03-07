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
			var rememeber = $("#remember").val();
			if (rememeber==true) {
				localStorage.setItem("hostAndPort", hostAndPort);
			}
			localStorage.setItem("storehostAndPort", remember);
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
		$("header nav").removeClass("hidden");
		websocketInstance.requestAllDocuments();
		websocketInstance.registerNotifier();
		websocketInstance.requestAllProperties();
	});

	$(storageInstance).on('onloadcomplete', function() {
		window.loadPage('scenelist');
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
