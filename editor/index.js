// show popup to get the new scene name
function newsceneName() {
	$('#newscenenamepopup').popup("open");
	$("#newscenename").delay(300).focus();
}
// call method to create scene
function newscene() {
	var name = escapeInputForJson($("#newscenename").val());
	if (name.length == 0)
		return;
	Scenes.create(name);
	$("#newscenename").val('');
	$('#newscenenamepopup').popup("close");
}
var selectedScenesCounter = 0;
function removeSelectedScenesAsk() {
	$(".currentrmscene").text(selectedScenesCounter);
	$('#removepopup').popup("open");
}

function removeSelectedScenes() {
	$('#removepopup').popup("close");
	$("#btnRemoveSelectedScenes").addClass("ui-disabled");
	selectedScenesCounter = 0;
	$("#scenelist").find("[data-selected='1']").removeClass("selectedSceneListItem").addClass("ui-disabled").each(function(index,elem){
		Scenes.delete(elem.getAttribute('data-sceneid'));
	});
}

function mark(sceneid) {
	var item = document.getElementById("li" + sceneid);
	if (item.getAttribute('data-selected')=="1") { // is selected: deselect
		$(item).removeClass("selectedSceneListItem");
		item.setAttribute('data-selected',"0");
		--selectedScenesCounter;
	} else {
		$(item).addClass("selectedSceneListItem");
		item.setAttribute('data-selected',"1");
		++selectedScenesCounter;
	}
	if (selectedScenesCounter)
		$("#btnRemoveSelectedScenes").removeClass("ui-disabled");
	else
		$("#btnRemoveSelectedScenes").addClass("ui-disabled");
}

function reloadpage() {
	window.location = window.location.href.replace( /#.*/, "");
}
$('#mainpage').live('pageinit', function(event) {
	$.mobile.loading( 'show', { theme: "b", text: "Lade Dokumente", textonly: false });
	//$('#splitviewcontainer').simplesplitview();
	templateSceneItem = Handlebars.compile($("#sceneitem-template").html());
	$("#btnRemoveSelectedScenes").addClass("ui-disabled");
	setscene(null);
	websocketInstance.reconnect();
	// focus to the name field
});
$(storageInstance).bind('onloadcomplete', function() {
	$.mobile.loading( 'hide' );
});
$(websocketInstance).bind('onclose', function() {
	$("#sceneservices").children().remove();
	$("#scenelist").children().remove();
	$.mobile.loading( 'hide' );
	//$("#noconnectionpopup").bind('popupafterclose',  function(event, ui) {console.log("pop"); $("#noconnectionpopup").popup('open'); }).popup('open');
	//$.mobile.changePage("#noconnectionpopup");
	$("#noconnectionpopup").popup('open');
});
$(storageInstance).bind('onscene', function(d, doc, removed) {
	// Remove the scene entry first
	//$("#scenelist").find("[data-sceneid='" + doc.id_ + "']").remove();
	$("#li" + doc.id_).remove();
	// if it just has been changed or wasn't in the list at all we need to add it
	if (!removed) {
		var entry = {"id":doc.id_, "name":doc.name, "counter":doc.temp_.counter};
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
});
function setscene(sceneid) {
	if (sceneid === null) {
		$("#btnAddEvent").addClass("ui-disabled");
		$("#btnAddCondition").addClass("ui-disabled");
		$("#btnAddAction").addClass("ui-disabled");
		return;
	}
	$("#btnAddEvent").removeClass("ui-disabled");
	$("#btnAddCondition").removeClass("ui-disabled");
	$("#btnAddAction").removeClass("ui-disabled");
	$(".currentscene").text(storageInstance.scenes[sceneid].name);
	$("#sceneservices").children().remove();
	CurrentScene.set(sceneid);
	var sceneDocuments = storageInstance.documentsForScene(sceneid);
	var templateSceneServiceItem = Handlebars.compile($("#sceneitem-service-template").html());
	for (var i=0;i < sceneDocuments.length;++i) {
		var doc = sceneDocuments[i];
		if (!doc)
			continue;
		var schema = storageInstance.schemaForDocument(doc);
		// create template key-value object "entry"
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
		
		var $elem = $(templateSceneServiceItem(entry));
		createParameterForm($elem.children('ul'), schema, doc);
		$elem.appendTo($("#sceneservices")).trigger("create");
		$elem.find(".btnsaveitem").addClass("ui-disabled");
		registerChangeNotifiers($elem.children('ul'));
	}
}

function saveSceneItem(docid, componentid, type) {
	var id = "#"+type+"_"+componentid+"_"+docid;
	id = id.replace(/\./g,"_");
	var $form = $(id);
	console.log("save from id: "+id,JSON.stringify($form.serializeObject(), null, 2))
	$.jGrowl("Save:"+JSON.stringify($form.serializeObject(), null, 2));
}
function removeSceneItem(docid, componentid, type) {
	$.jGrowl("Saved!");
}