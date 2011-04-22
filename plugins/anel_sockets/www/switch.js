function InitPlugin(pluginid, sectionname, $section) {
	var datamodel = modelstorage.getModel("anel.io.value");
	var namemodel = modelstorage.getModel("anel.io.name");

	if (datamodel && namemodel) {
		function getName(key) {
			var count = namemodel.count();
			for(i=0;i<count;++i) {
				var item = namemodel.getItem(i);
				if (item.channel == key) return item.name;
			}
			return key;
		}
		
		function itemHtmlFunction(data) {
			return getName(data.channel) + ": "+data.value;
		}
		
		function itemCreationFunction() {
			return $("<li></li>");
		}

		var $rootelement = $("<ul></ul>");
		var listview = new ListView($rootelement, itemHtmlFunction, itemCreationFunction, datamodel);
		$section.append($rootelement);
	}
}