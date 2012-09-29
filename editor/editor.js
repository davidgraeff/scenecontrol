var selectedScenesCounter = 0;
var sceneRenameFlag = false;
var templateSceneItem;
var templateSceneServiceItem;

// All button click events
$(document).one('pageinit', function() {
	if (!websocketInstance.connected)
		window.location = 'index.html';
	$(document).off(".editorpage");
	$.mobile.loading( 'show', { theme: "b", text: "Verarbeite Dokumente", textonly: false });
	templateSceneServiceItem = Handlebars.compile($("#sceneitem-service-template").html());
	templateSceneItem = Handlebars.compile($("#sceneitem-template").html());
	$("#btnRemoveSelectedScenes").addClass("ui-disabled");
	setscene(null);
	for (var index in storageInstance.scenes) {
		sceneChanged(storageInstance.scenes[index], false);
	}
	$('#editor').one('pageshow', function(event) {
		$.mobile.loading( 'hide' );
	});
	
	$('#configpage').one('pagebeforechange', function(event) {
		$(document).off(".editorpage");
	});
	
	$('#btnGotoConfig').on('click.editorpage', function() {
		$.mobile.changePage('config.html', {transition: 'slide'});
	});
	
	////////////////// ADD SCENE ITEM //////////////////
	$('#btnAddSceneItem').on('click.editorpage', function() {
		$("#sceneitem_type_event").trigger("change");
		$('#newsceneitempopup').popup("open");
	});
	
	$("#sceneitem_type input[name='type_']").on('change.editorpage', function() {
		var type = $("#sceneitem_type input[name='type_']:checked").val();
		
		if (type=="event")
			$(".newsceneitemtitle").text("Neues Ereignis");
		else if (type=="condition")
			$(".newsceneitemtitle").text("Neue Bedingung");
		else if (type=="action")
			$(".newsceneitemtitle").text("Neue Aktion");
		
		$("#sceneitem_type").val(type);
		var plugins = storageInstance.componentIDsFromConfigurationsByType(type);
		$("#sceneitem_componentid").empty();
		for (var i =0;i<plugins.length;++i) {
			var cid = plugins[i].componentid_;
			var instance = plugins[i].instanceid_;
			$("#sceneitem_componentid").append("<option data-instance='"+instance+"' value='"+cid+"'>"+cid+":"+instance+"</option>");
		}
		$("#sceneitem_componentid").prop("selectedIndex",0);
		$("#sceneitem_componentid").trigger("change");
	});
	
	$('#sceneitem_componentid').on('change.editorpage', function() {
		var type = $("#sceneitem_type input[name='type_']:checked").val();
		var pluginid = $("#sceneitem_componentid").val();
		$("#sceneitem_method").empty();
		var schemas = storageInstance.schemaForPlugin(pluginid, type);
		for (var i =0;i<schemas.length;++i) {
			$("#sceneitem_method").append("<option value='"+schemas[i].method_+"'>"+schemas[i].name+"</option>");
		}
		$("#sceneitem_method").prop("selectedIndex",0);
		$("#sceneitem_method").trigger("change");
	});
	
	$('#btnConfirmSceneitem').on('click.editorpage', function() {
		var type = $("#sceneitem_type input[name='type_']:checked").val();
		var componentid = $("#sceneitem_componentid").val();
		var instanceid = $("#sceneitem_componentid option:selected").first().attr("data-instance");
		var method = $("#sceneitem_method").val();
		
		var doc = {type_:type, componentid_:componentid, instanceid_:instanceid, method_:method, sceneid_: CurrentScene.id, id_:"GENERATEGUID"};
		// Simulate a new document
		storageInstance.documentChanged(doc, false);
		storageInstance.notifyDocumentChange(doc, false, true);
	});
	
	////////////////// SCENE TAGS //////////////////
	$('#btnChangeTags').on('click.editorpage', function() {
		$("#scenetags").val(CurrentScene.getTags());
		$('#scenetagspopup').popup("open");
	});
	
	$('#btnConfirmScenetags').on('click.editorpage', function() {
		var tags = $("#scenetags").val().split(",");
		$("#li"+CurrentScene.id).addClass("ui-disabled");
		CurrentScene.setTags(tags);
		$('#scenetagspopup').popup("close");
	});

	////////////////// SET AND SELECT SCENES //////////////////
	$("#scenelist").on('click.editorpage', '.btnSetScene', function() {
		var sceneid = $(this).parent().parent().parent().attr("data-sceneid");
		setscene(sceneid);
	});
	
	$("#scenelist").on('click.editorpage', '.btnSelectScene', function() {
		var $scenelistentry = $(this).parent();
		if ($scenelistentry.attr('data-selected')=="1") { // is selected: deselect
			$scenelistentry.removeClass("selectedSceneListItem");
			$scenelistentry.attr('data-selected', "0");
			--selectedScenesCounter;
		} else {
			$scenelistentry.addClass("selectedSceneListItem");
			$scenelistentry.attr('data-selected', "1");
			++selectedScenesCounter;
		}
		if (selectedScenesCounter)
			$("#btnRemoveSelectedScenes").removeClass("ui-disabled");
		else
			$("#btnRemoveSelectedScenes").addClass("ui-disabled");
	});
	
	////////////////// REMOVE SCENES //////////////////
	$('#btnRemoveSelectedScenes').on('click.editorpage', function() {
		$(".currentrmscene").text(selectedScenesCounter);
		$('#removepopup').popup("open");
	});
	
	$('#btnRemoveSelectedScenesConfirm').on('click.editorpage', function() {
		$('#removepopup').popup("close");
		$("#btnRemoveSelectedScenes").addClass("ui-disabled");
		selectedScenesCounter = 0;
		$("#scenelist").find("[data-selected='1']").removeClass("selectedSceneListItem").addClass("ui-disabled").each(function(index,elem){
			var sceneid = elem.getAttribute('data-sceneid');
			var doc = storageInstance.scenes[storageInstance.unqiueSceneID(sceneid)];
			Document.remove(doc);
		});
	});
	
	////////////////// SCENE NAME AND ADD SCENE //////////////////
	$('#btnChangeSceneName').on('click.editorpage', function() {
		sceneRenameFlag = true;
		$("#newscenename").val(CurrentScene.getName());
		$('#newscenenamepopup').popup("open");
		$("#newscenename").delay(300).focus();
	});
	
	$('#btnAddScene').on('click.editorpage', function() {
		sceneRenameFlag = false;
		$('#newscenenamepopup').popup("open");
		$("#newscenename").delay(300).focus();
	});
	
	$('#btnConfirmScenename').on('click.editorpage', function() {
		var name = escapeInputForJson($("#newscenename").val());
		if (name.length == 0) {
			$.jGrowl("Name nicht gültig. A-Za-z0-9 sind zugelassen!");
			return;
		}
		if (!sceneRenameFlag) { // create new scene
			Document.createScene(name);
		} else { // just rename old scene
			$("#li"+CurrentScene.id).addClass("ui-disabled");
			CurrentScene.rename(name);
		}
		$("#newscenename").val('');
		$('#newscenenamepopup').popup("close");
	});
	
	////////////////// SAVE SCENE ITEM AND REMOVE SCENE ITEM //////////////////
	$("#righteditpanel").on('click.editorpage', '.btnSaveSceneitem', function() {
		var $form = $(this).parent().parent().parent();
		var docid = $form.attr("data-id")
		var doctype = $form.attr("data-type")
		var componentid = $form.attr("data-componentid")
		
		var obj = storageInstance.addEssentialDocumentData(docid, doctype, componentid, {});
		var objdata = serializeForm($form);
		
		if (obj && objdata.invalid.length==0) {
			obj = jQuery.extend(true, obj, objdata.data);
			$.jGrowl("Saving ...");
			$form.find("input,select,label,textarea,.btnSaveSceneitem").addClass("ui-disabled");
			Document.change(obj);
		} else {
			$.jGrowl("Incomplete: "+doctype+"."+componentid);
			console.warn("Incomplete", obj, objdata)
		}
	});
	
	$("#righteditpanel").on('click.editorpage', '.btnRemoveSceneitem', function() {
		var $form = $(this).parent().parent().parent();
		var docid = $form.attr("data-id")
		var doctype = $form.attr("data-type")
		var componentid = $form.attr("data-componentid")
		
		var obj = storageInstance.addEssentialDocumentData(docid, doctype, componentid, {});
		if (!obj)
			return;
		$.jGrowl("Removing...");
		$form.find("input,select,label,textarea,.btnSaveSceneitem,.btnRemoveSceneitem").addClass("ui-disabled");
		Document.remove(obj);
	});
});


$(storageInstance).on('onscene.editorpage', function(d, flags) {
	sceneChanged(flags.doc, flags.removed);
});

$(storageInstance).on('onevent.editorpage', function(d, flags) {
	if (flags.doc.sceneid_ != CurrentScene.id)
		return;
	
	if (!flags.removed)
		addSceneItem(flags.doc, flags.temporary);
	else
		removeSceneItem(flags.doc, flags.temporary);
});

$(storageInstance).on('oncondition.editorpage', function(d, flags) {
	if (flags.doc.sceneid_ != CurrentScene.id)
		return;
	
	if (!flags.removed)
		addSceneItem(flags.doc, flags.temporary);
	else
		removeSceneItem(flags.doc, flags.temporary);
});

$(storageInstance).on('onaction.editorpage', function(d, flags) {
	if (flags.doc.sceneid_ != CurrentScene.id)
		return;
	
	if (!flags.removed)
		addSceneItem(flags.doc, flags.temporary);
	else
		removeSceneItem(flags.doc, flags.temporary);
});

function sceneChanged(doc, removed) {
	var $entries = $("#scenelist").find("li[data-sceneid='"+doc.id_+"']");
	if (removed && $entries.length) {
		// Remove the scene entry
		$entries.remove();
	} else {
		// Remove the scene entry (TODO: Replace at same place)
		$entries.remove();
		// if it just has been changed or wasn't in the list at all we need to add it
		var entry = {"sceneid":doc.id_, "name":doc.name, "counter":storageInstance.sceneItemCount(doc.id_)};
		var categories = doc.categories;
		// No categories: Add it to the general categories
		if (categories.length == 0)
			$("#scenelist_cat_none").after(templateSceneItem(entry));
		// one or multiple categories: add 
			else
				for (var i=0;i<categories.length;++i) {
					var trimmedCat = categories[i].replace(" ", "_").replace(".", "_");
					if (!$("#scenelist_cat_"+trimmedCat).length)
						$("#scenelist_cat_none").before('<li id="scenelist_cat_'+trimmedCat+'" data-role="list-divider">'+categories[i]+'</li>');
					$("#scenelist_cat_"+trimmedCat).after(templateSceneItem(entry));
				}
	}
	$('#scenelist').listview('refresh');
	CurrentScene.checkscene(doc.id_, removed);
	if (!CurrentScene.isValid()) {
		setscene(null);
	}
}


function removeSceneItem(doc, temporary) {
	var formid = doc.type_+"_"+doc.componentid_+"_"+doc.id_;
	formid = formid.replace(/\./g,"_");
	$('#'+formid).hide("slow", function(){ $(this).remove(); })
}

function addSceneItem(doc, temporary) {
	if (!doc)
		return;
	var schema = storageInstance.schemaForDocument(doc);
	var formid = doc.type_+"_"+doc.componentid_+"_"+doc.id_;
	formid = formid.replace(/\./g,"_");
	var entry = {"id":doc.id_, "componentid": doc.componentid_, "type": doc.type_, "formid": formid, "name": doc.componentid_+"."+doc.instanceid_}
	if (schema == null) {
		schema = {
			parameters:{
				raw:{name:"Schemalose Daten",type:"rawdoc"}
			}
		};
		// add to template key-value object "entry"
		entry.subname = doc.method_;
		entry.typetheme= "d";
	} else {
		// add to template key-value object "entry"
		entry.subname = schema.name;
		entry.typetheme= (doc.type_=="action")?"a":((doc.type_=="condition")?"b":"c");
	}
	if (temporary) {
		entry.typetheme= "d";
		entry.name +=" (Neu, nicht gespeichert)"
	}
	
	var $elem = $(templateSceneServiceItem(entry));
	var ok = createParameterForm($elem.children('ul'), schema, doc);
	
	if (temporary && !ok)
		return;
	
	// add or replace in dom
	if ($('#'+formid).length) { // already there
		$('#'+formid).replaceWith($elem);
	} else {
		$elem.appendTo($("#sceneservices"))
	}
	
	// post-dom-adding stuff
	$elem.trigger("create");
	
	if (!temporary)
		$elem.find(".btnSaveSceneitem").addClass("ui-disabled");
	
	if (ok)
		registerChangeNotifiers($elem.children('ul'), function($ulBase) {$ulBase.find(".btnSaveSceneitem").removeClass("ui-disabled");});
}

function setscene(sceneid) {
	if (sceneid === null) {
		$("#btnAddSceneItem").addClass("ui-disabled");
		$("#btnChangeTags").addClass("ui-disabled");
		$("#btnChangeSceneName").addClass("hidden");
		$("#helptexteditor").removeClass("hidden");
		$("#sceneservices").children().remove();
		$(".currentscene").text("Keine Szene ausgewählt");
		return;
	}
	$("#btnAddSceneItem").removeClass("ui-disabled");
	$("#btnChangeTags").removeClass("ui-disabled");
	$("#btnChangeSceneName").removeClass("hidden");
	$("#helptexteditor").addClass("hidden");
	
	$(".currentscene").text(storageInstance.scenes[storageInstance.unqiueSceneID(sceneid)].name);
	$("#sceneservices").children().remove();
	CurrentScene.set(sceneid);
	var sceneDocuments = storageInstance.documentsForScene(sceneid);
	var templateSceneServiceItem = Handlebars.compile($("#sceneitem-service-template").html());
	for (var i=0;i < sceneDocuments.length;++i) {
		addSceneItem(sceneDocuments[i]);
	}
}
