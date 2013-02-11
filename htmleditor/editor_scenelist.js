/**
 * Editor Module - JS file for editor.html
 * Uses modules: Canvas, sceneCanvas, Storage, Document
 */
(function (window) {
	"use strict";

	var selectedScenesCounter = 0;
	var sceneRenameFlag = false;
	var templateSceneItem;

	// All button click events
	$(document).one('pageinit', function() {
		$.mobile.loading( 'show', { theme: "b", text: "Verarbeite Dokumente", textonly: false });

		templateSceneItem = Handlebars.compile($("#sceneitem-template").html());
		$("#btnRemoveSelectedScenes").addClass("ui-disabled");
		SceneUIHelper.setscene(null);
		for (var index in storageInstance.scenes) {
			SceneUIHelper.sceneChanged(storageInstance.scenes[index], false);
		}

		////////////////// SET AND SELECT SCENES //////////////////
		$("#scenelist").on('click.editorpage', '.btnSetScene', function() {
			var sceneid = $(this).parent().parent().parent().attr("data-sceneid");
			SceneUIHelper.setscene(sceneid);
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
				var doc = storageInstance.scenes[sceneid];
				Document.remove(doc);
			});
		});
		
		////////////////// ADD SCENE //////////////////
		$('#btnAddScene').on('click.editorpage', function() {
			SceneUIHelper.sceneRenameFlag = false;
			$('#newscenenamepopup').popup("open");
			$("#newscenename").delay(300).focus();
		});
		
	});

	$(storageInstance).on('onscene.editorpage', function(d, flags) {
		SceneUIHelper.sceneChanged(flags.doc, flags.removed);
	});
	
	
	window.SceneUIHelper = {
		sceneRenameFlag: false,
 
		sceneChanged: function(doc, removed) {
			var $entries = $("#scenelist").find("li[data-sceneid='"+doc.id_+"']");
			if (removed) {
				// Remove the scene entry
				if ($entries.length) $entries.remove();
			} else {
				// Remove the scene entry (TODO: Replace at same place)
				$entries.remove();
				// if it just has been changed or wasn't in the list at all we need to add it
				var entry = {"sceneid":doc.id_, "name":doc.name, "counter":doc.v.length};
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
				SceneUIHelper.setscene(null);
			}
		},

		setscene: function(sceneid) {
			var scene = storageInstance.scenes[sceneid];
			if (sceneid === null || scene === null) {
				$("#btnAddSceneItem").addClass("ui-disabled");
				$("#btnChangeTags").addClass("ui-disabled");
				$("#btnChangeSceneName").addClass("ui-disabled");
				$("#helptexteditor").removeClass("hidden");
		// 		$("#sceneservices").children().remove();
				$(".currentscene").text("Keine Szene ausgewählt");
				return;
			}
			$("#btnAddSceneItem").removeClass("ui-disabled");
			$("#btnChangeTags").removeClass("ui-disabled");
			$("#btnChangeSceneName").removeClass("ui-disabled");
			$("#helptexteditor").addClass("hidden");

			$(".currentscene").text(scene.name);
			$("#sceneservices").children().remove();
			CurrentScene.set(sceneid);

			var canvas = document.getElementById('canvas');
			sceneCanvas.load(scene, canvas.width, canvas.getContext('2d'));
			sceneCanvas.draw(canvas);
		}
	};
})(window);
