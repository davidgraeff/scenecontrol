var currentPluginid;
var templateConfigItem;
var templateConfigServiceItem;

// All button click events
$(document).one('pageinit', function() {
	if (!websocketInstance.connected)
		window.location = 'index.html';
	$(document).off(".configpage");
	$.mobile.loading( 'show', { theme: "b", text: "Verarbeite Dokumente", textonly: false });
	// precompile templates
	templateConfigServiceItem = Handlebars.compile($("#configitem-service-template").html());
	templateConfigItem = Handlebars.compile($("#configitem-template").html());
	setPlugin(null);
	for (var i = 0;i < storageInstance.plugins.length; ++i) {
		var pluginid = storageInstance.plugins[i].replace(":","_");
		$("#li" + pluginid).remove();
		var entry = {"id":pluginid, "name":pluginid, "counter":storageInstance.configurationsForPlugin(pluginid).length};
		$("#pluginlistheader").after(templateConfigItem(entry));
	}
	$('#configlist').listview('refresh');
	
	$('#configpage').one('pagebeforechange', function(event) {
		$(document).off(".configpage");
	});
	
	$('#configpage').on('pageshow.configpage', function(event) {
		$.mobile.loading( 'hide' );
	});
	
	$('#btnGotoScenes').on('click.configpage', function() {
		$.mobile.changePage('editor.html', {transition: 'slide',reverse:true});
	});
	
	$('#btnAddConfiguration').on('click.configpage', function() {
		$("#newconfiginstance").val('');
		$("#configinstancepopup").popup("open");
		$("#newconfiginstance").delay(300).focus();
	});
	
	$('#btnConfirmConfiginstance').on('click.configpage', function() {
		var instanceid = escapeInputForJson($("#newconfiginstance").val());
		if (instanceid.length == 0 || currentPluginid.length == 0) {
			$.jGrowl("Name nicht gültig. A-Za-z0-9 sind zugelassen!");
			return;
		}
		
		if (storageInstance.configInstanceIDUsed(currentPluginid, instanceid)) {
			$.jGrowl("Bereits verwendet!");
			return;
		}
		Document.createConfig(instanceid, currentPluginid);
		$('#configinstancepopup').popup("close");
	});
	
	$("#rightconfigpanel").on('click.configpage', '.btnSaveConfigitem', function() {
		var $form = $(this).parent().parent().parent();
		var configid = $form.attr("data-configid")
		var componentid = $form.attr("data-componentid")
		
		var obj = storageInstance.addEssentialDocumentData(configid, "configuration", componentid, {});
		var objdata = serializeForm($form);
		
		if (obj && objdata.invalid.length==0) {
			obj = jQuery.extend(true, obj, objdata.data);
			$.jGrowl("Saving: "+configid);
			$form.find("input,select,label,textarea,.btnSaveConfigitem").addClass("ui-disabled");
			Document.change(obj);
		} else {
			$.jGrowl("Incomplete: "+configid);
		}
	});
	
	$("#rightconfigpanel").on('click.configpage', '.btnRemoveConfigitem', function() {
		console.log("khbsdfk")
		var $form = $(this).parent().parent().parent();
		var configid = $form.attr("data-configid")
		var componentid = $form.attr("data-componentid")
		
		var obj = storageInstance.addEssentialDocumentData(configid, "configuration", componentid, {});
		if (!obj)
			return;
		$.jGrowl("Removing...");
		$form.find("input,select,label,textarea,.btnSaveConfigitem,.btnRemoveConfigitem").addClass("ui-disabled");
		Document.remove(obj);
	});
});

$(storageInstance).on('onconfiguration.configpage', function(d, doc, removed) {
	if (doc.componentid_==currentPluginid) {
		if (!removed)
			addConfigItem(doc);
		else
			removeConfigItem(doc);
	}
});

function removeConfigItem(doc) {
	$('#'+doc.id_).remove();
}

function addConfigItem(doc) {
	if (!doc)
		return;
	var schema = storageInstance.schemaForDocument(doc);
	if (schema == null) {
		schema = {
			parameters:{
				instanceid_:{name:"Plugin-instanz",type:"string"},
				raw:{name:"Schemalose Daten",type:"rawdoc"}
			}
		};
	}
	var formid = doc.id_;
	formid = formid.replace(/\./g,"_");
	var entry = {"configid":doc.id_, "componentid": doc.componentid_,"formid": formid, "name": doc.componentid_, "subname": doc.instanceid_, "typetheme":"d"};
	var $elem = $(templateConfigServiceItem(entry));
	var ok = createParameterForm($elem.children('ul'), schema, doc);
	
	// add or replace in dom
	if ($('#'+formid).length) { // already there
		console.log("replace", currentPluginid);
		$('#'+formid).replaceWith($elem);
	} else {
		$elem.appendTo($("#configservices"))
	}
	
	// post-dom-adding stuff
	$elem.trigger("create");
	$elem.find(".btnSaveConfigitem").addClass("ui-disabled");
	$('textarea').keyup();
	if (ok)
		registerChangeNotifiers($elem.children('ul'), function($ulBase) {$ulBase.find(".btnSaveConfigitem").removeClass("ui-disabled");});
}

function setPlugin(pluginid) {
	if (pluginid === null) {
		$("#btnAddConfiguration").addClass("ui-disabled");
		return;
	}
	$("#btnAddConfiguration").removeClass("ui-disabled");
	$(".currentplugin").text(pluginid);
	$("#configservices").children().remove();
	
	currentPluginid = pluginid;
	
	var sceneDocuments = storageInstance.configurationsForPlugin(pluginid);
	for (var i=0;i < sceneDocuments.length;++i) {
		addConfigItem(sceneDocuments[i]);
	}
}
