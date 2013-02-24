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
			var $entries = $("#scenelist").find("li[data-sceneid='"+doc.id_+"']");
			if (removed==true) {
				// Remove the scene entry
				if ($entries.length) $entries.remove();
			} else {
				// Remove the scene entry (TODO: Replace at same place)
				$entries.remove();
				// if it just has been changed or wasn't in the list at all we need to add it
				var entry = {"sceneid":doc.id_, "name":doc.name, "counter":doc.v.length};
				var categories = doc.categories;
				// No categories: Add it to the general categories
				if (categories.length == 0) {
					$("#scenelist_cat_none").handlebarsAfter("#sceneitem-template", entry);
				}
				// one or multiple categories: add 
				else
					for (var i=0;i<categories.length;++i) {
						var trimmedCat = categories[i].replace(" ", "_").replace(".", "_");
						if (!$("#scenelist_cat_"+trimmedCat).length)
							$("#scenelist_cat_none").before('<li id="scenelist_cat_'+trimmedCat+'" data-role="list-divider">'+categories[i]+'</li>');
						$("#scenelist_cat_"+trimmedCat).handlebarsAfter("#sceneitem-template", entry);
					}
			}
			//TODO $('#scenelist').listview('refresh');
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
		}
	};
	

	// All button click events
	$(function() {
		$.mobile.loading( 'show', { theme: "b", text: "Verarbeite Dokumente", textonly: false });

		$("#btnRemoveSelectedScenes").addClass("ui-disabled");
		
		for (var index in storageInstance.scenes) {
			SceneUIHelper.sceneChanged(storageInstance.scenes[index], false);
		}

		////////////////// SET AND SELECT SCENES //////////////////
		$("#scenelist").on('click.editorpage', '.btnSetScene', function() {
			var sceneid = $(this).parent().parent().parent().attr("data-sceneid");
			CurrentScene.set(sceneid);
			loadPage('sceneitems');
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
				websocketInstance.remove(doc);
			});
		});
		
		////////////////// ADD SCENE //////////////////
		$('#btnAddScene').on('click.editorpage', function() {
			$('#newscenepopup').popup("open");
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
			$('#newscenepopup').popup("close");
		});
	});

})(window);
