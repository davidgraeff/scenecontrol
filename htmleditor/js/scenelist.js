/**
 * Editor Module - JS file for editor.html
 * Uses modules: Canvas, sceneCanvas, Storage, Document
 */
(function (window) {
	"use strict";

	var selectedScenesCounter = 0;

	$(storageInstance).on('onscene.sceneitemspage', function(d, flags) {
		if (flags.temporary)
			return;
		console.log("onscene", flags);
		SceneUIHelper.sceneChanged(flags.doc, flags.removed);
	});
	
	window.SceneUIHelper = {
		sceneLastAdded: null,
		sceneChanged: function(doc, removed) {
			var $entries = $("#scenelists").find("*[data-sceneid='"+doc.id_+"']");
			if (removed==true) {
				// Remove the scene entry
				if ($entries.length) $entries.remove();
			} else {
				// Remove the scene entry (TODO: Replace at same place)
				$entries.remove();
				// if it just has been changed or wasn't in the list at all we need to add it
				var entry = {"sceneid":doc.id_, "name":doc.name, "counter":doc.v.length};
				// No categories: Add it to the general categories
				var categories = $.extend({}, doc).categories;
				if (categories.length == 0) {
					categories.push("Unkategorisiert");
				}
				// add to all categories
				for (var i=0;i<categories.length;++i) {
					var catid = "scenelist_cat_" + categories[i].replace(" ", "_").replace(".", "_");
					if (!$("#"+catid).length) {
						// add not existing category holder
						var colornumber = $('.sceneList').length % 10;
						var cat_entry = {"catid": catid, "name":categories[i], "colorclass":"q"+colornumber};
						$("#scenelists").handlebarsAppend("#sceneitem-cat-template", cat_entry);
					}
					var $cat = $("#"+catid);
					entry.colorclass = $cat.attr("data-colorclass");
					$cat.handlebarsAppend("#sceneitem-template", entry);
				}
// 				this.renderDocImage(doc);
			}

			if (SceneUIHelper.sceneLastAdded) {
				var $entries = $("#scenelist").find("li[data-sceneid='"+doc.id_+"']");
				if (!removed && SceneUIHelper.sceneLastAdded == doc.name) {
					$.mobile.silentScroll($entries.first().offset().top);
					var properties = {
						paddingTop:    '10px',
						paddingBottom: '10px'
					};

					$entries.pulse(properties,{duration : 1000, pulses:2});
				}
				SceneUIHelper.sceneLastAdded = null;
			}
		}/*,
		renderDocImage: function(scene) {
			// prepare canvas'
			var canvasDrawBig = document.createElement("canvas");
			canvasDrawBig.width = 800; canvasDrawBig.height = 600;
			// load scene
			var sc = new sceneCanvas();
			sc.setCanvas(canvasDrawBig);
			sc.load(scene);
			// store into image and resize
			var image = new Image();
			image.onload = function() {
				var canvasDrawSmall = document.createElement("canvas");
				canvasDrawSmall.width = 160; canvasDrawSmall.height = 100;
				var canvasDrawSmallCtx = canvasDrawSmall.getContext("2d");
				canvasDrawSmallCtx.drawImage(image, 0, 0, canvasDrawSmall.width, canvasDrawSmall.height);
				// unload
				sc.unload();
				sc = null;
				// return data
				//console.log("pic", canvasDrawSmall.toDataURL('image/jpeg'));
				$("*[data-sceneid='"+scene.id_+"']").css("background-image", "url('"+ canvasDrawSmall.toDataURL()+"')");
				
			};
			image.src = sc.getImage();
		}*/
	};
	

	// All button click events
	$(function() {
		$.mobile.loading( 'show', { theme: "b", text: "Verarbeite Dokumente", textonly: false });

		$("#btnRemoveSelectedScenes").addClass("disabled");
		
		for (var index in storageInstance.scenes) {
			SceneUIHelper.sceneChanged(storageInstance.scenes[index], false);
		}

		////////////////// SET AND SELECT SCENES //////////////////
		$(".btnSetScene").on('click.editorpage', function() {
			var sceneid = $(this).parent().attr("data-sceneid");
			if (sceneid == undefined) {
				console.warn("no data-sceneid!");
				return;
			}

			CurrentScene.set(sceneid);
			loadPage('sceneitems');
		});
		
		$(".btnSelectScene").on('click.editorpage', function() {
			var $scenelistentry = $(this).parent().parent().parent();
			if ($scenelistentry.attr('data-selected')=="1") { // is selected: deselect
				$scenelistentry.removeClass("selectedSceneListItem").addClass("unselectedSceneListItem");
				$scenelistentry.attr('data-selected', "0");
				--selectedScenesCounter;
			} else {
				$scenelistentry.addClass("selectedSceneListItem").removeClass("unselectedSceneListItem");
				$scenelistentry.attr('data-selected', "1");
				++selectedScenesCounter;
			}
			if (selectedScenesCounter)
				$("#btnRemoveSelectedScenes").removeClass("disabled");
			else
				$("#btnRemoveSelectedScenes").addClass("disabled");
		});
		
		////////////////// REMOVE SCENES //////////////////
		$('#btnRemoveSelectedScenes').on('click.editorpage', function() {
			$(".currentrmscene").text(selectedScenesCounter);
			$('#removepopup').modal("show");
		});
		
		$('#btnRemoveSelectedScenesConfirm').on('click.editorpage', function() {
			$('#removepopup').modal("hide");
			$("#btnRemoveSelectedScenes").addClass("disabled");
			selectedScenesCounter = 0;
			$("#scenelist").find("[data-selected='1']").removeClass("selectedSceneListItem").addClass("disabled").each(function(index,elem){
				var sceneid = elem.getAttribute('data-sceneid');
				var doc = storageInstance.scenes[sceneid];
				websocketInstance.remove(doc);
			});
		});
		
		////////////////// ADD SCENE //////////////////
		$('#btnAddScene').on('click.editorpage', function() {
			$('#newscenepopup').modal("show");
			$("#newscenename").delay(300).focus();
		});
		
		$('#btnConfirmNewScenename').on('click.sceneitemspage', function() {
			var name = storageInstance.escapeInputForJson($("#newscenename").val());
			if (name.length == 0) {
				$.jGrowl("Name nicht gültig. A-Za-z0-9 sind zugelassen!");
				return;
			}
			SceneUIHelper.sceneLastAdded = name;
			websocketInstance.createScene(name);
			$("#newscenename").val('');
			$('#newscenepopup').modal("hide");
		});
	});

})(window);
