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
	
	function drawbackground($elem, catid) {
		// determine position
		var $root = $elem.parent().parent(); // tbody element
		var xpos = $elem.index(), ypos = $elem.parent().index();
		var top, right, left, bottom;
		if (xpos==0) {
			top=1;bottom=1;left=1;
		} else {
			right = $root.children().eq(ypos).children().eq(xpos+1);
			//console.log("rigth", $root.children().eq(ypos).children().eq(xpos+1));
			right = right.children().length?false:true;
			top = (ypos==0) || ($root.children().eq(ypos-1).hasClass(catid)?0:1);
			bottom = $root.children().eq(ypos+1);
			bottom = (bottom.children().eq(xpos).children().length==0) || (bottom.hasClass(catid)?false:true);
			// left
			if (xpos==1 && !top) {
				left = 1;
			}
		}
		
		// prepare canvas'
		var canvasDrawBG = document.createElement("canvas");
		canvasDrawBG.width = $elem.width()+2;
		canvasDrawBG.height = $elem.height()+1;
		var ctx = canvasDrawBG.getContext("2d");
		
		// path for background
		var x = 0, y = 0, width = canvasDrawBG.width, height=canvasDrawBG.height, radius=5;
		ctx.beginPath();
		ctx.moveTo(x + radius, y);
		ctx.lineTo(x + width - radius, y);
		ctx.quadraticCurveTo(x + width, y, x + width, y + radius);
		ctx.lineTo(x + width, y + height - radius);
		ctx.quadraticCurveTo(x + width, y + height, x + width - radius, y + height);
		ctx.lineTo(x + radius, y + height);
		ctx.quadraticCurveTo(x, y + height, x, y + height - radius);
		ctx.lineTo(x, y + radius);
		ctx.quadraticCurveTo(x, y, x + radius, y);
		ctx.closePath();   
		
		ctx.fillStyle = '#fafafa';
		ctx.fill();
		
		// path for line
		ctx.beginPath();
		
		if (top) {
			ctx.moveTo(x + (left?radius:0), y); // top line
			ctx.lineTo(x + width - (right?radius:0), y); 
		}
		
		if (top && right) {
			ctx.moveTo(x + width - radius, y); // top right
			ctx.quadraticCurveTo(x + width, y, x + width, y + radius);
		}
		
		if (right) {
			ctx.moveTo(x + width, y + (top?radius:0)); // right line
			ctx.lineTo(x + width, y + height - (bottom?radius:0));
		}
		
		if (bottom && right) {
			ctx.moveTo(x + width, y + height - radius); // bottom right
			ctx.quadraticCurveTo(x + width, y + height, x + width - radius, y + height);
		}
		
		if (bottom) {
			ctx.moveTo(x + width - (right?radius:0), y + height); // bottom line
			ctx.lineTo(x + (left?radius:0), y + height);
		}
		
		if (bottom && left) {
			ctx.moveTo(x + radius, y + height); // bottom left
			ctx.quadraticCurveTo(x, y + height, x, y + height - radius);
		}
		
		if (left) {
			ctx.moveTo(x, y + height- (bottom?radius:0)); // left line
			ctx.lineTo(x, y + (top?radius:0));
		}
		
		if (top && left) {
			ctx.moveTo(x, y + radius); // top left
			ctx.quadraticCurveTo(x, y, x + radius, y);
		}
		
		ctx.strokeStyle = 'black';		
		ctx.stroke();
		
		$elem.css("background-image", "url('"+ canvasDrawBG.toDataURL()+"')");
		
	}
	
	function drawCategoryBackground(catid) {
		$("tr."+catid).children().each(function(index, elem) {
			var $elem = $(elem);
			if ($elem.children().length)
				drawbackground($elem, catid);
		});
		
	}
	
	function getNextFreeCellInCat(catid, catname) {
		// try find the cat
		if (!$("#"+catid).length) {
			// add not existing category holder
			//var colornumber = $('.sceneList').length % 10;
			var cat_entry = {"catid": catid, "name": catname};
			var $tr = $("<tr class='firstcatrow "+catid+"'></tr>").appendTo($("#scenelists"));
			var $td = $("<td></td>").appendTo($tr);
			$td.handlebarsAppend("#sceneitem-cat-template", cat_entry);
			drawbackground($td);
			$td = $("<td></td>").appendTo($tr);
			for (var i=2;i<7;++i)
				$tr.append("<td></td>");
			return $td;
		}
		// find free td
		var $catTRs = $("tr."+catid);
		var $elem = $catTRs.find("td:empty").first();
		if ($elem.length)
			return $elem;
		// else add tr
		var $tr = $("<tr class='"+catid+"'></tr>").insertAfter($catTRs.last());
		$tr.append("<td> </td>");
		$elem = $("<td></td>").appendTo($tr);
		for (var i=2;i<7;++i)
			$tr.append("<td></td>");
		return $elem;
	}

	window.SceneUIHelper = {
		drag: null,
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
					var $nextFreeCell = getNextFreeCellInCat(catid, categories[i]);
					$nextFreeCell.handlebarsAppend("#sceneitem-template", entry);
					drawCategoryBackground(catid);
					if (this.drag)
						this.drag.enableDrag(true, $nextFreeCell.get(0));
				}
			}

			if (SceneUIHelper.sceneLastAdded) {
				if (!removed && SceneUIHelper.sceneLastAdded == responseid) {
					var $entries = $("#scenelist").find("*[data-sceneid='"+doc.id_+"']");
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
			// init drag lib
			this.drag = REDIPS.drag;
			var that = this;
			REDIPS.drag.style.borderEnabled = 'none';
			REDIPS.drag.style.borderDisabled = 'none';
			REDIPS.drag.dropMode = 'overwrite';
			
			this.drag.event.dropped = function () {
				var $destelem = $(that.drag.obj).parent();
				var sceneid = $(that.drag.obj).attr("data-sceneid");
				
			}
			this.drag.event.deleted = function () {
				var $destelem = $(that.drag.obj).parent();
				var sceneid = $(that.drag.obj).attr("data-sceneid");
				console.log("remove", sceneid);
				var doc = storageInstance.getDocument("scene",sceneid);
				if (!doc) {
					console.warn("btnRemoveSelectedScenesConfirm: No id!");
					return false;
				}
				websocketInstance.write(api.manipulatorAPI.remove(doc));
				return true;
			}
			this.drag.event.clicked = function (currentCell) {
				$(".trash").removeClass("disabled");
				return true;
			}
			this.drag.event.finish = function () {
				$(".trash").addClass("disabled");
			}
			this.drag.event.droppedBefore = function (targetCell) {
				return false;
			}
			this.drag.init('canvascontainer');
			$(".trash").addClass("disabled");
		}
	};
	
	window.SceneUIHelper.load();
	
	// All button click events
	$(function() {
		////////////////// SET AND SELECT SCENES //////////////////
		$('#scenelists').on('dblclick.editorpage',".btnSetScene", function() {
			var sceneid = $(this).parent().attr("data-sceneid");
			if (sceneid == undefined) {
				console.warn("no data-sceneid!");
				return;
			}

			CurrentScene.set(sceneid);
			loadPage('sceneitems');
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
