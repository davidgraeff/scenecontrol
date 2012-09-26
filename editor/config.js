// call method to create scene
function newscene() {
	var name = escapeInputForJson($("#newscenename").val());
	if (name.length == 0)
		return;
	Scenes.create(name);
	$("#newscenename").val('');
	$('#newscenenamepopup').popup("close");
}

function reloadpage() {
	window.location = window.location.href.replace( /#.*/, "");
}

$('#mainpage').live('pageinit', function(event) {
	$.mobile.loading( 'show', { theme: "b", text: "Lade Dokumente", textonly: false });
	//$('#splitviewcontainer').simplesplitview();
	templateSceneItem = Handlebars.compile($("#sceneitem-template").html());
	setPlugin(null);
	websocketInstance.reconnect();
});

$(storageInstance).bind('onloadcomplete', function() {
	$.mobile.loading( 'hide' );
});

$(websocketInstance).bind('onclose', function() {
	$("#sceneservices").children().remove();
	$("#scenelist").children().remove();
	$.mobile.loading( 'hide' );
	$("#noconnectionpopup").popup('open');
});

$(storageInstance).bind('onplugins', function(d) {
	// Remove the scene entry first
	for (var i = 0;i < storageInstance.plugins.length; ++i) {
		var pluginid = storageInstance.plugins[i].replace(":","_");
		$("#li" + pluginid).remove();
		var entry = {"id":pluginid, "name":pluginid, "counter":storageInstance.configurationsForPlugin(pluginid).length};
		$("#scenelist_cat_none").after(templateSceneItem(entry));
	}
	$('#scenelist').listview('refresh');
	setPlugin(null);
});
function setPlugin(pluginid) {
	if (pluginid === null) {
		$("#btnAddConfiguration").addClass("ui-disabled");
		return;
	}
	$("#btnAddConfiguration").removeClass("ui-disabled");
	$(".currentscene").text(pluginid);
	$("#sceneservices").children().remove();
	
	var sceneDocuments = storageInstance.configurationsForPlugin(pluginid);
	var templateSceneServiceItem = Handlebars.compile($("#sceneitem-service-template").html());
	for (var i=0;i < sceneDocuments.length;++i) {
		var doc = sceneDocuments[i];
		if (!doc)
			continue;
		var schema = storageInstance.schemaForDocument(doc);
		if (schema == null) {
			schema = {
				parameters:{
					raw:{name:"Schemalose Daten",type:"rawdoc"}
				}
			};
		}
		var entry = {"configid":doc.id_, "componentid": doc.componentid_, "name": doc.componentid_, "subname": doc.instanceid_, "typetheme":"d"};
		var $elem = $(templateSceneServiceItem(entry));
		createParameterForm($elem.children('ul'), schema, doc);
		$elem.appendTo($("#sceneservices")).trigger("create");
		$elem.find(".btnsaveitem").addClass("ui-disabled");
		$('textarea').keyup();
		registerChangeNotifiers($elem.children('ul'));
	}
}
function saveConfig(configid, componentid) {
	$.jGrowl("Saved!");
}
function removeConfig(configid, componentid) {
	$.jGrowl("Saved!");
}