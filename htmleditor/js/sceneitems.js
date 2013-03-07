/**
 * Editor Module - JS file for editor.html
 * Uses modules: Canvas, sceneCanvas, Storage, Document
 */
(function (window) {
	"use strict";

	// get scene
	var scene = storageInstance.scenes[CurrentScene.id];
	if (scene==null) {
		//window.location = 'index.html';
		console.warn("FAIL: Scene not set",CurrentScene.id);
		loadPage('scenelist');
	}
	
	// prepare page
	$("#maincontent").off(".sceneitemspage");
	var SceneItemCreator = {type_:null,componentid_:null, instanceid_:null};

	// prepare canvas object
	var sceneCanvasObject = new sceneCanvas();
	sceneCanvasObject.setCanvas(document.getElementById('canvas'));
	
	$(sceneCanvasObject).on('itemtriggered.sceneitemspage', function(e, data) {
		SceneItemsUIHelper.showSceneItemDialog(data, false);
	});
	
	$('#btnBack').on('click.sceneitemspage', function() {
		loadPage('scenelist');
	});
	
	$('#btnExecute').on('click.sceneitemspage', function() {
		websocketInstance.runcollection(scene.id_);
	});
		
	////////////////// ADD SCENE ITEM //////////////////
	$('#btnAddSceneItem').on('click.sceneitemspage', function() {
		$("#sceneitem_type_event").trigger("change");
		$('#newsceneitempopup').modal("show");
	});
	
	$(".newsceneitempopuptype").on('click.sceneitemspage', function() {
		var type = $(this).val();
		console.log(type);
		
		if (type=="event")
			$(".newsceneitemtitle").text("Neues Ereignis");
		else if (type=="condition")
			$(".newsceneitemtitle").text("Neue Bedingung");
		else if (type=="action")
			$(".newsceneitemtitle").text("Neue Aktion");
		
		SceneItemCreator.type_ = type;
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
	
	$('#sceneitem_componentid').on('change.sceneitemspage', function() {
		SceneItemCreator.componentid_ = $("#sceneitem_componentid").val();
		$("#sceneitem_method").empty();
		var schemas = storageInstance.schemaForPlugin(SceneItemCreator.componentid_, SceneItemCreator.type_);
		for (var i =0;i<schemas.length;++i) {
			$("#sceneitem_method").append("<option value='"+schemas[i].method_+"'>"+schemas[i].name+"</option>");
		}
		$("#sceneitem_method").prop("selectedIndex",0);
		$("#sceneitem_method").trigger("change");
	});
	
	$('#btnConfirmSceneitem').on('click.sceneitemspage', function() {
		var instanceid = $("#sceneitem_componentid option:selected").first().attr("data-instance");
		var method = $("#sceneitem_method").val();
		
		var doc = {type_:SceneItemCreator.type_, componentid_:SceneItemCreator.componentid_, instanceid_:instanceid, method_:method, sceneid_: CurrentScene.id, id_:"GENERATEGUID"};
		websocketInstance.createSceneItem(CurrentScene.id, doc);
	});
	
	////////////////// SCENE TAGS //////////////////
	$('#btnChangeTags').on('click.sceneitemspage', function() {
		$("#scenetags").val(CurrentScene.getTags());
		$('#scenetagspopup').modal("show");
	});
	
	$('#btnConfirmScenetags').on('click.sceneitemspage', function() {
		var tags = $("#scenetags").val().split(",");
		$("#li"+CurrentScene.id).addClass("disabled");
		CurrentScene.setTags(tags);
		$('#scenetagspopup').modal("hide");
	});
	
	////////////////// SCENE NAME //////////////////
	$('#btnChangeSceneName').on('click.sceneitemspage', function() {
		SceneUIHelper.sceneRenameFlag = true;
		$("#editscenename").val(CurrentScene.getName());
		$('#editscenenamepopup').modal("show");
		$("#editscenename").delay(300).focus();
	});
	
	$('#btnConfirmEditScenename').on('click.sceneitemspage', function() {
		var name = storageInstance.escapeInputForJson($("#editscenename").val());
		if (name.length == 0) {
			$.jGrowl("Name nicht gültig. A-Za-z0-9 sind zugelassen!");
			return;
		}
		$("#li"+CurrentScene.id).addClass("disabled");
		CurrentScene.rename(name);
		$("#editscenename").val('');
		$('#editscenenamepopup').modal("hide");
	});
	
	////////////////// SAVE SCENE ITEM AND REMOVE SCENE ITEM //////////////////
	$("#btnSaveSceneItem").on('click.sceneitemspage', function() {
		var $form = $("#sceneitemform");

		var objdata = CurrentSceneItem.serializeForm($form);
		
		if (CurrentSceneItem.item && objdata.invalid.length==0) {
			var obj = jQuery.extend(true, CurrentSceneItem.item, objdata.data);
			$.jGrowl("Saving ...");
			$form.find("input,select,label,textarea,.btnSaveSceneitem").addClass("disabled");
			sceneCanvasObject.disable(CurrentSceneItem.item);
			sceneCanvasObject.draw();
			websocketInstance.updateDocument(obj);
			$('#sceneitemedit').modal("hide");
		} else {
			$.jGrowl("Incomplete: "+CurrentSceneItem.item.type_+"."+CurrentSceneItem.item.componentid_);
			console.warn("Incomplete", CurrentSceneItem.item, objdata)
		}
	});
	
	$("#btnRemoveSceneItem").on('click.sceneitemspage', function() {
		var $form = $("#sceneitemform");

		$.jGrowl("Removing...");
		//$form.find("input,select,label,textarea,.btnSaveSceneitem,.btnRemoveSceneitem").addClass("disabled");
		
		sceneCanvasObject.disable(CurrentSceneItem.item);
		sceneCanvasObject.draw();
		
		websocketInstance.removeSceneItem(CurrentScene.id, CurrentSceneItem.item);
		$('#sceneitemedit').modal("hide");
	});

	// for getting propteries from the server and provide them in the edit dialogs
	$(storageInstance).off(".sceneitemspage"); 
	$(storageInstance).on('onnotification.sceneitemspage', function(d, doc) {
		if (CurrentSceneItem.item==null)
			return;
		CurrentSceneItem.notification($("#sceneitemform"),doc);
	});

	$(storageInstance).on('onscene.sceneitemspage', function(d, flags) {
		CurrentScene.checkscene(flags.doc.id_, flags.removed);
		if (!CurrentScene.isValid()) {
			loadPage('scenelist');
		} else {
			SceneItemsUIHelper.load(flags.doc);
		}
	});
	
	var SceneItemsUIHelper = {
		showSceneItemDialog: function(doc, temporary) {
			if (!doc)
				return;

			CurrentSceneItem.set(doc, temporary);
			
			$('#sceneitemname').text(CurrentSceneItem.name);
			$('#sceneitemsubname').text(CurrentSceneItem.subname);
			$('#sceneitemproperties').empty();
			var ok = CurrentSceneItem.createParameterForm($('#sceneitemproperties'));
			
			if (temporary && !ok)
				return;

			if (!temporary) {
				$("#btnSaveSceneItem").addClass("disabled");
				$("#btnRemoveSceneItem").removeClass("disabled");
			} else {
				$("#btnRemoveSceneItem").addClass("disabled");
				$("#btnSaveSceneItem").removeClass("disabled");
			}
			
			if (ok)
				CurrentSceneItem.registerChangeNotifiers($('#sceneitemproperties'), function() {$("#btnSaveSceneItem").removeClass("disabled");});
			
			$('#sceneitemedit').modal("show");
		},
 
		load: function(scene) {
			$("#btnAddSceneItem").removeClass("disabled");
			$("#btnChangeTags").removeClass("disabled");
			$("#btnChangeSceneName").removeClass("disabled");
			
			$(".currentscene").text(scene.name);
			CurrentScene.set(scene.id_);
			
			sceneCanvasObject.load(scene);
			$.jGrowl("Szene geladen: "+scene.name,{life:800,animateClose:null});
		}
	};
	
	// init
	SceneItemsUIHelper.load(scene);
})(window);
