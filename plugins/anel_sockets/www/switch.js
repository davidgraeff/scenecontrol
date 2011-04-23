function InitPlugin(pluginid, sectionname, $section) {
	var datamodel = modelstorage.getModel("anel.io.value");
	var namemodel = modelstorage.getModel("anel.io.name");
	
	if (datamodel && namemodel) {
		$.getCss(pluginid+"/"+sectionname+".css");

		function getName(key) {
			var count = namemodel.count();
			for(i=0;i<count;++i) {
				var item = namemodel.getItem(i);
				if (item.channel == key) return item.name;
			}
			return key;
		}
		
		function itemHtmlFunction(modelitem, domitem) {
			domitem.removeClass("anelsockets_activated anelsockets_deactivated");
			if (modelitem.value)
				domitem.addClass("anelsockets_activated");
			else
				domitem.addClass("anelsockets_deactivated");
			return getName(modelitem.channel);
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

		var $rootelement = $('<ul class="anelsockets"></ul>');
		var listview = new ListView($rootelement, itemHtmlFunction, itemCreationFunction, datamodel);
		$section.append($rootelement);
	}
}