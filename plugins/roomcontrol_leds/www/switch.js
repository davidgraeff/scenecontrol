function InitPlugin(pluginid, sectionname, $section) {
	var datamodel = modelstorage.getModel("led.value");
	var namemodel = modelstorage.getModel("led.name");

	var $rootelement = $('<div class="roomcontrolleds"></div>');
	$section.append($rootelement);

	/* fake daten */
	datamodel.reset({"__key":"channel","id":"led.value"});
	datamodel.change({"channel":"led1","value":120,"id":"led.value"});
	namemodel.reset({"__key":"channel","id":"led.name"});
	namemodel.change({"channel":"led1","name":"Led 1","id":"led.name"});
	
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
			var itemText = domitem.itemText;
			var itemSlider = domitem.itemSlider;
			itemText.text(getName(modelitem.channel));
			itemSlider.slider("value", modelitem.value);
			return domitem;
		}
		
		function changeled(channel, value) {
			sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"ledvalue_absolut","channel":channel,"value":value});
		}
		
		function itemCreationFunction(modelitem) {
			var item = $('<div class="led"></div>');
			var itemText = $('<div class="ledtext" />');
			var itemSlider = $('<div class="ledslider" />');
			item.itemText = itemText;
			item.itemSlider = itemSlider;
			item.append(itemSlider);
			item.append(itemText);
			
			itemSlider.slider({min: 0, max: 255, value: 0, orientation: 'horizontal'});
			itemSlider.bind( "slide", function(event, ui) {changeled(modelitem.channel, ui.value);});
			return item;
		}

		var listview = new AbstractView($rootelement, itemChangeFunction, itemCreationFunction, datamodel);
	}
	
	function modelAvailable(event, modelid, modeldata) {
		if (modelid == "led.value") datamodel = modeldata;
		else if (modelid == "led.name") namemodel = modeldata;
		modelsAvailable();
	}
	
	$(modelstorage).bind('modelAvailable', modelAvailable);
	modelsAvailable();
}