/**
 * Editor Module - JS file for editor.html
 * Uses modules: Canvas, sceneCanvas, Storage, Document
 */
(function (window) {
	"use strict";

	// get scene
	var scene = storageInstance.getDocument("scene",CurrentScene.id);
	if (scene==null) {
		//window.location = 'index.html';
		console.warn("FAIL: Scene not set",CurrentScene.id);
		loadPage('scenelist');
	}
	
	// prepare page
	$("#maincontent").off(".sceneitemspage");

	// prepare canvas object
	var sceneCanvasObject = new sceneCanvas();
	
	$(sceneCanvasObject).on('itemtriggered.sceneitemspage', function(e, data) {
		SceneItemsUIHelper.showSceneItemDialog(data);
	});
	$(sceneCanvasObject).on('itemremove.sceneitemspage', function(e, data) {
		SceneItemsUIHelper.removeItem(data);
	});
	$(sceneCanvasObject).on('itemexecute.sceneitemspage', function(e, data) {
		websocketInstance.runaction(data);
	});
	
	$('#btnBack').on('click.sceneitemspage', function() {
		loadPage('scenelist');
	});
	
	$('#btnExecute').on('click.sceneitemspage', function() {
		websocketInstance.runcollection(scene.id_);
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
	$('#btnAddSceneItem').on('click.sceneitemspage', function() {
		SceneItemsUIHelper.showSceneItemDialog();
	});
	
	$(".newsceneitempopuptype").on('click.sceneitemspage', function() {
		SceneItemsUIHelper.SceneItemDialogTypeSelected($(this).val());
	});
	
	$('#sceneitem_componentid').on('change.sceneitemspage', function() {
		var instanceid = $("#sceneitem_componentid option:selected").first().attr("data-instance");
		SceneItemsUIHelper.SceneItemDialogComponentSelected($("#sceneitem_componentid").val(), instanceid);
	});
	
	$('#sceneitem_method').on('change.sceneitemspage', function() {
		SceneItemsUIHelper.SceneItemDialogMethodSelected($("#sceneitem_method").val());
	});
	
	$("#btnSaveSceneItem").on('click.sceneitemspage', function() {
		SceneItemsUIHelper.saveSceneitem();
	});
	
	$("#btnRemoveSceneItem").on('click.sceneitemspage', function() {
		SceneItemsUIHelper.removeItem(CurrentSceneItem.item);
		$('#newsceneitempopup').modal("hide");
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
		lastAdded: null,
		ackTimeoutTimer: null,
 
		SceneItemDialogTypeSelect: function(type) {
			CurrentSceneItem.item.type_ = type;
			
			if (type=="event")
				$("#sceneitem_type_event").button('toggle');
			else if (type=="condition")
				$("#sceneitem_type_condition").button('toggle');
			else if (type=="action")
				$("#sceneitem_type_action").button('toggle');
			
			this.SceneItemDialogTypeSelected(type);
		},
 
		SceneItemDialogTypeSelected: function(type) {
			CurrentSceneItem.item.type_ = type;
			
			if (type=="event")
				$(".newsceneitemtitle").text("Neues Ereignis");
			else if (type=="condition")
				$(".newsceneitemtitle").text("Neue Bedingung");
			else if (type=="action")
				$(".newsceneitemtitle").text("Neue Aktion");
			
			var plugins = storageInstance.componentIDsFromConfigurationsByType(type);
			$("#sceneitem_componentid").empty();
			var selectedIndex = 0;
			for (var i =0;i<plugins.length;++i) {
				var cid = plugins[i].componentid_;
				var instance = plugins[i].instanceid_;
				var name = storageInstance.getComponentName(cid);
				$("#sceneitem_componentid").append("<option data-instance='"+instance+"' value='"+cid+"'>"+name+" ("+instance+")</option>");
				if (CurrentSceneItem.item.componentid_ == cid)
					selectedIndex = i;
			}
			$("#sceneitem_componentid").prop("selectedIndex", selectedIndex);
			$("#sceneitem_componentid").trigger("change");
		},
		
		SceneItemDialogComponentSelected: function(componentid, instanceid) {
			CurrentSceneItem.item.componentid_ = componentid;
			CurrentSceneItem.item.instanceid_ = instanceid;
			
			//console.log(CurrentSceneItem.item.componentid_);
			$("#sceneitem_method").empty();
			var selectedIndex = 0;
			var schemas = storageInstance.schemaForPlugin(CurrentSceneItem.item.componentid_, CurrentSceneItem.item.type_);
			for (var i =0;i<schemas.length;++i) {
				var method = schemas[i].method_;
				$("#sceneitem_method").append("<option value='"+method+"'>"+schemas[i].name+"</option>");
				if (CurrentSceneItem.item.method_ == method)
					selectedIndex = i;
			}
			$("#sceneitem_method").prop("selectedIndex", selectedIndex);
			$("#sceneitem_method").trigger("change");
		},
		
		SceneItemDialogMethodSelected: function(method) {
			CurrentSceneItem.item.method_ = method;
			
			$("#btnSaveSceneItem").text(CurrentSceneItem.item.id_?"Speichern":"Erstellen");
			
			var $form = $("#sceneitemform");
			$form.empty();
			var ok = CurrentSceneItem.createParameterForm($form);
			
			if (ok)
				CurrentSceneItem.registerChangeNotifiers($form, function() {$("#btnSaveSceneItem").removeClass("disabled");});
			
		},
 
		saveSceneitem: function() {
			if (!CurrentSceneItem.item.method_ || !CurrentSceneItem.item.componentid_ || !CurrentSceneItem.item.instanceid_)
				return;
			
			// get scene item specialized data
			var $form = $("#sceneitemform");
			var objdata = CurrentSceneItem.serializeForm($form);
			// get scene item basic data
			
			if (CurrentSceneItem.item && objdata.invalid.length==0) {
				$("#btnSaveSceneItem").addClass("disabled");
				$form.find("input,select,label,textarea").attr("disabled", true);
				
				var obj = jQuery.extend(true, CurrentSceneItem.item, objdata.data);
				sceneCanvasObject.disable(CurrentSceneItem.item);
				SceneItemsUIHelper.lastAdded = Math.round(Math.random()*10000);
				var cmd;
				if (CurrentSceneItem.item.id_)
					cmd = api.manipulatorAPI.updateSceneItem(CurrentSceneItem.item);
				else
					cmd = api.manipulatorAPI.createSceneItem(CurrentSceneItem.item);
				
				//console.log(api.addRequestID(cmd, SceneItemsUIHelper.lastAdded));
				websocketInstance.write(api.addRequestID(cmd, SceneItemsUIHelper.lastAdded));
				
				$(storageInstance).on("on"+CurrentSceneItem.item.type_+".sceneitemsdialog", function(d, flags) {
					if (flags.reponseid == SceneItemsUIHelper.lastAdded)
						SceneItemsUIHelper.closeSceneItemDialog(true);
				});
				
				SceneItemsUIHelper.ackTimeoutTimer = setTimeout(function() {SceneItemsUIHelper.closeSceneItemDialog(false); },1500);
				
			} else {
				$.jGrowl("Incomplete: "+CurrentSceneItem.item.type_+"."+CurrentSceneItem.item.componentid_);
				console.warn("Incomplete", CurrentSceneItem.item, objdata)
			}
		},
 
		closeSceneItemDialog: function(success) {
			if (SceneItemsUIHelper.ackTimeoutTimer) {
				clearTimeout(SceneItemsUIHelper.ackTimeoutTimer);
				SceneItemsUIHelper.ackTimeoutTimer = null;
			}
			$(storageInstance).off(".sceneitemsdialog");
			
			if (success)
				$.jGrowl("Successfully saved ...");
			else
				$.jGrowl("Failed to save!");
			$('#newsceneitempopup').modal("hide");
		},
 
		showSceneItemDialog: function(doc) {
			var $form = $("#sceneitemform");
			$form.empty();
			$("#sceneitem_componentid").empty();
			$("#sceneitem_method").empty();
			
			if (!doc) { // new item
				$("#sceneitem_type_event").trigger("change");
				$('#btnRemoveSceneItem').hide();
				CurrentSceneItem.set({sceneid_: CurrentScene.id}, true);
				this.SceneItemDialogTypeSelect("event");
			} else { // edit item
				CurrentSceneItem.set(jQuery.extend(true, {sceneid_: CurrentScene.id}, doc), false);
				this.SceneItemDialogTypeSelect(doc.type_);
				$('#btnRemoveSceneItem').show();
			}

			$("#btnSaveSceneItem").addClass("disabled");
			$('#newsceneitempopup').modal("show");
		},
 
		removeItem: function(sceneItem) {
			$.jGrowl("Removing...");
			sceneCanvasObject.disable(sceneItem);
			websocketInstance.write(api.manipulatorAPI.removeSceneItem(sceneItem));
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
