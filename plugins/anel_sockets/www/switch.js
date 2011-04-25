function InitPlugin(pluginid, sectionname, $section) {
	var datamodel = modelstorage.getModel("anel.io.value");
	var namemodel = modelstorage.getModel("anel.io.name");
	
	var $rootelement = $('<ul class="anelsockets"></ul>');
	$section.append($rootelement);

	function modelsAvailable() {
		if (!datamodel || !namemodel) return;
		$.getCss(pluginid+"/"+sectionname+".css");

		function getName(key) {
			var count = namemodel.count();
			for(i=0;i<count;++i) {
				var item = namemodel.getItem(i);
				if (item.channel == key) return item.name;
			}
			return key;
		}
		
		function itemChangeFunction(domitem, modelitem) {
			if (modelitem.value)
				domitem.removeClass("anelsockets_deactivated").addClass("anelsockets_activated");
			else
				domitem.removeClass("anelsockets_activated").addClass("anelsockets_deactivated");
			return domitem.text(getName(modelitem.channel));
		}
		
		function tooglevalue(key) {
			sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"iovalue_toogle","channel":key});
		}
		
		function itemCreationFunction(modelitem) {
			var item = $('<li class="anelsockets"></li>');
			item.key = modelitem.channel;
			item.click( function() { tooglevalue(item.key); });
			return item;
		}

		var listview = new ListView($rootelement, itemChangeFunction, itemCreationFunction, datamodel);
	}
	
	function modelAvailable(event, modelid, modeldata) {
		if (modelid == "anel.io.value") datamodel = modeldata;
		else if (modelid == "anel.io.name") namemodel = modeldata;
		modelsAvailable();
	}
	
	$(modelstorage).bind('modelAvailable', modelAvailable);
	modelsAvailable();
}