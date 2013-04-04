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
		SceneUIHelper.sceneChanged(flags.doc, flags.removed);
	});
	
	window.SceneUIHelper = {
		sceneLastAdded: null,
		sceneChanged: function(doc, removed, responseid) {
// 			console.log("onscene", doc);
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
				if (!removed && SceneUIHelper.sceneLastAdded == responseid) {
					$.mobile.silentScroll($entries.first().offset().top);
					var properties = {
						paddingTop:    '10px',
						paddingBottom: '10px'
					};

					$entries.pulse(properties,{duration : 1000, pulses:2});
				}
				SceneUIHelper.sceneLastAdded = null;
			}
		},
		load: function() {
			storageInstance.forEveryDocument("scene", function(scenedoc) {
				SceneUIHelper.sceneChanged(scenedoc, false);
			});
		}
	};
	
	window.SceneUIHelper.load();
	
	// All button click events
	$(function() {
		$.mobile.loading( 'show', { theme: "b", text: "Verarbeite Dokumente", textonly: false });

		$("#btnRemoveSelectedScenes").addClass("disabled");
		
		////////////////// SET AND SELECT SCENES //////////////////
		$('#scenelists').on('click.editorpage',".btnSetScene", function() {
			var sceneid = $(this).parent().attr("data-sceneid");
			if (sceneid == undefined) {
				console.warn("no data-sceneid!");
				return;
			}

			CurrentScene.set(sceneid);
			loadPage('sceneitems');
		});
		
		$('#scenelists').on('click.editorpage',".btnSelectScene", function() {
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
			console.log("Remove");
			$("#scenelists").find("[data-selected='1']").removeClass("selectedSceneListItem").addClass("disabled").each(function(index,elem){
				var sceneid = elem.getAttribute('data-sceneid');
				var doc = storageInstance.getDocument("scene",sceneid);
				if (!doc) {
					console.warn("btnRemoveSelectedScenesConfirm: No id!");
					return;
				}
				websocketInstance.write(api.manipulatorAPI.remove(doc));
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
			SceneUIHelper.sceneLastAdded = name+Math.round(Math.random()*10000);
			websocketInstance.write(api.addRequestID(api.manipulatorAPI.createScene(name), SceneUIHelper.sceneLastAdded));
			$("#newscenename").val('');
			$('#newscenepopup').modal("hide");
		});
	});

})(window);
