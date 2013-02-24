/**
 * Editor Module - JS file for editor.html
 * Uses modules: Canvas, sceneCanvas, Storage, Document
 */
(function (window) {
	"use strict";

	var scene = storageInstance.scenes[CurrentScene.id];
	if (scene==null) {
		//window.location = 'index.html';
		console.warn("FAIL: Scene not set",CurrentScene.id);
		loadPage('scenelist');
	}
	$("#maincontent").off(".sceneitemspage");
			
	var shift= null;
	var originalMouse= null;
	var scrollMaxX= null;
	var scrollMaxY= null;
	var movingObject =  null;
	var SceneItemCreator = {type_:null,componentid_:null, instanceid_:null};
	
	var canvas = document.getElementById('canvas');
	var $canvas = $(canvas);
	
	
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
			sceneCanvas.disable(CurrentSceneItem.item);
			sceneCanvas.draw();
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
		
		sceneCanvas.disable(CurrentSceneItem.item);
		sceneCanvas.draw();
		
		websocketInstance.removeSceneItem(CurrentScene.id, CurrentSceneItem.item);
		$('#sceneitemedit').modal("hide");
	});


	function resizecanvas() {
		canvas.width=parseInt($canvas.css('width'));
		canvas.height = window.innerHeight - 140; // - header
		if (!sceneCanvas.init) {
			SceneItemsUIHelper.load(scene);
			sceneCanvas.init = true;
		} else
			sceneCanvas.draw();
	}

	$canvas.mousedown(function(e) {
		if (e.which!=1)
			return;
		
		// set focus to canvas (to get key strokes)
		$canvas.attr("tabindex", "0");
		// mouse position
		var mouse = {'x': e.pageX - $canvas.offset().left,'y': e.pageY - $canvas.offset().top};

		sceneCanvas.selectedObject = sceneCanvas.selectObject(mouse.x, mouse.y);
		
		movingObject = false;
		if (sceneCanvas.selectedObject != null) {
			if (shift && sceneCanvas.selectedObject instanceof Node) {
				$canvas.css('cursor', 'pointer');
				sceneCanvas.currentLink = new TemporaryLink(sceneCanvas.selectedObject, mouse);
			} else {
				$canvas.css('cursor', 'move');
				movingObject = true;
				if (sceneCanvas.selectedObject.setMouseStart) {
					sceneCanvas.selectedObject.setMouseStart(mouse.x, mouse.y);
				}
			}
		} else {
			originalMouse = mouse;
			$canvas.css('cursor', 'move');
		}
		sceneCanvas.draw();
		
		if (document.activeElement == canvas) {
			// disable drag-and-drop only if the canvas is already focused
			return false;
		} else {
			// otherwise, let the browser switch the focus away from wherever it was
			return true;
		}
	});
	
	canvas.ondblclick = function(e) {
		var mouse = {'x': e.pageX - $canvas.offset().left,'y': e.pageY - $canvas.offset().top};
		sceneCanvas.selectedObject = sceneCanvas.selectObject(mouse.x, mouse.y);
		
		if (sceneCanvas.selectedObject == null)
			return;
		
		if (sceneCanvas.selectedObject instanceof Node && sceneCanvas.selectedObject.enabled) {
			SceneItemsUIHelper.showSceneItemDialog(sceneCanvas.selectedObject.data, false);
		} else if (sceneCanvas.selectedObject instanceof Link) {
			sceneCanvas.removeLink(sceneCanvas.selectedObject);
			sceneCanvas.draw();
			sceneCanvas.store();
		}
	};
	
	$canvas.mousemove(function(e) {
		if (sceneCanvas.currentLink != null) {
			var mouse = {'x': e.pageX - $canvas.offset().left,'y': e.pageY - $canvas.offset().top}; 
			var targetNode = sceneCanvas.selectObject(mouse.x, mouse.y);
			if (!(targetNode instanceof Node)) {
				targetNode = null;
			}
			
			if (sceneCanvas.selectedObject != null) {
				if (targetNode != null) {
					sceneCanvas.currentLink = new Link(sceneCanvas.selectedObject, targetNode);
				} else {
					sceneCanvas.currentLink = new TemporaryLink(sceneCanvas.selectedObject.closestPointOnShape(mouse.x, mouse.y), mouse);
				}
			}
			sceneCanvas.draw();
		} else if (movingObject) {
			sceneCanvas.selectedObject.setAnchorPoint(e.pageX - $canvas.offset().left, e.pageY - $canvas.offset().top);
			sceneCanvas.snapNode();
			sceneCanvas.draw();
		} else if (originalMouse != null) { // move nodes
			var mouse = {'x': e.pageX - $canvas.offset().left,'y': e.pageY - $canvas.offset().top}; 
			sceneCanvas.moveNodes(mouse.x - originalMouse.x,
									mouse.y - originalMouse.y);
			sceneCanvas.draw();
			originalMouse = mouse;
		}
	});
	
	$canvas.mouseout(function () {
		if (movingObject)
			sceneCanvas.store();
		sceneCanvas.currentLink = null;
		sceneCanvas.draw();
		movingObject = false;
		originalMouse = null;
		$canvas.css('cursor', 'default');
	});
	
	$(document).mouseup(function(e) {
		if (sceneCanvas.currentLink != null) {
			if (!(sceneCanvas.currentLink instanceof TemporaryLink)) {
				sceneCanvas.selectedObject = sceneCanvas.currentLink;
				sceneCanvas.links.push(sceneCanvas.currentLink);
				movingObject = true; // reuse as indication for sceneCanvas.store()
			}
			sceneCanvas.currentLink = null;
			sceneCanvas.draw();
		}
		if (movingObject || originalMouse)
			sceneCanvas.store();
		
		movingObject = false;
		originalMouse = null;
		$canvas.css('cursor', 'default');
	});
	
	$canvas.keydown(function(event) {
		var key = event.which;
		if (key == 16) {
			shift = true;
		}
	});
	
	$canvas.keyup(function(event) {
		var key = event.which;
		
		if (key == 16) {
			shift = false;
		}
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
			
			$("#sceneitemform").trigger("create");
			$("#sceneitemform").trigger("change");
			
			$('#sceneitemedit').modal("show");
		},
 
		load: function(scene) {
			$("#btnAddSceneItem").removeClass("disabled");
			$("#btnChangeTags").removeClass("disabled");
			$("#btnChangeSceneName").removeClass("disabled");
			
			$(".currentscene").text(scene.name);
			CurrentScene.set(scene.id_);
			
			sceneCanvas.load(scene);
			$.jGrowl("Szene geladen: "+scene.name,{life:800,animateClose:null});
		}
	};
	
	// init
	sceneCanvas.init = false;
	sceneCanvas.setCanvas(canvas);
	$(window).resize(function() {resizecanvas();});
	setTimeout( resizecanvas, 500 );
})(window);
