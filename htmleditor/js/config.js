/**
 * Plugin Configuration Module - JS file for plugininstances.html
 * Uses modules: Storage, Document
 */
(function (window) {
	"use strict";
	
	var currentPluginid;
	$("#maincontent").off(".configpage");
	$.mobile.loading( 'show', { theme: "b", text: "Verarbeite Dokumente", textonly: false });
	// precompile templates
	var templateConfigItem = Handlebars.compile($("#configitem-template").html());
	

	$('#btnBack').on('click.configpage', function() {
		loadPage('scenelist');
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
	
	$('#configlist').on('click.configpage', "button", function() {
		var pluginid = $(this).attr("data-pluginid");
		setPlugin(pluginid);
	});
	
	$(".btnSaveConfigitem").on('click.configpage', function() {
		var $form = $(this).parent();
		var configid = $form.attr("data-configid")
		var componentid = $form.attr("data-componentid")
		
		var obj = storageInstance.addEssentialDocumentData(configid, "configuration", componentid, {});
		var objdata = serializeForm($form);
		
		if (obj && objdata.invalid.length==0) {
			obj = jQuery.extend(true, obj, objdata.data);
			$.jGrowl("Saving: "+configid);
			$form.find("input,select,label,textarea,.btnSaveConfigitem").addClass("disabled");
			Document.change(obj);
		} else {
			$.jGrowl("Incomplete: "+configid);
		}
	});
	
	$(".btnRemoveConfigitem").on('click.configpage', function() {
		console.log("khbsdfk")
		var $form = $(this).parent();
		var configid = $form.attr("data-configid")
		var componentid = $form.attr("data-componentid")
		
		var obj = storageInstance.addEssentialDocumentData(configid, "configuration", componentid, {});
		if (!obj)
			return;
		$.jGrowl("Removing...");
		$form.find("input,select,label,textarea,.btnSaveConfigitem,.btnRemoveConfigitem").addClass("disabled");
		Document.remove(obj);
	});

	$(storageInstance).on('onconfiguration.configpage', function(d, flags) {
		if (flags.doc.componentid_==currentPluginid) {
			if (!flags.removed)
				addConfigItem(flags.doc, flags.temporary);
			else
				removeConfigItem(flags.doc, flags.temporary);
		}
	});

	function removeConfigItem(doc, temporary) {
		var formid = doc.id_;
		formid = formid.replace(/\./g,"_");
		$('#'+formid).remove();
	}

	function addConfigItem(doc, temporary) {
		if (!doc)
			return;
		var schema = storageInstance.schemaForDocument(doc);
// 		if (schema == null) {
			schema = {
				parameters:{
					instanceid_:{name:"Plugin-instanz",type:"string"},
					raw:{name:"Schemalose Daten",type:"rawdoc"}
				}
			};
// 		}
		var formid = doc.id_;
		formid = formid.replace(/\./g,"_");
		var entry = {"configid":doc.id_, "componentid": doc.componentid_,"formid": formid, "name": doc.componentid_, "subname": doc.instanceid_};
		
		$('#'+formid).remove();
		$("#configservices").handlebarsAppend("#configitem-service-template", entry);
				
		// post-dom-adding stuff
		$('#'+formid +" .btnSaveConfigitem").addClass("disabled");
		$('#'+formid +' textarea').keyup();
// 		if (ok)
// 			registerChangeNotifiers($('#'+formid +' ul'), function($ulBase) {$ulBase.find(".btnSaveConfigitem").removeClass("disabled");});
	}

	function setPlugin(pluginid) {
		if (pluginid === null) {
			$("#btnAddConfiguration").addClass("disabled");
			return;
		}
		$("#btnAddConfiguration").removeClass("disabled");
		$(".currentplugin").text(pluginid);
		$("#configservices").children().remove();
		
		currentPluginid = pluginid;
		
		var sceneDocuments = storageInstance.configurationsForPlugin(pluginid);
		for (var i=0;i < sceneDocuments.length;++i) {
			addConfigItem(sceneDocuments[i]);
		}
	}
	
	setPlugin(null);
	for (var i = 0;i < storageInstance.plugins.length; ++i) {
		var pluginid = storageInstance.plugins[i].replace(":","_");
		$("#li" + pluginid).remove();
		var entry = {"id":pluginid, "name":pluginid, "counter":storageInstance.configurationsForPlugin(pluginid).length};
		$("#configlist").append(templateConfigItem(entry));
	}
	
})(window);