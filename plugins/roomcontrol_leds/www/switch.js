function RoomPlugin(pluginid, sectionname, $section) {
	var that = this;
	this.$rootelement = $('<div class="roomcontrolleds"></div>');
	this.datamodel;
	this.namemodel;
	this.listview;
	
	this.load = function() {
		if (!that.$rootelement) return;
		$section.append(that.$rootelement);
		$.getCss(pluginid+"/"+sectionname+".css");
		that.listview = new AbstractView(that.$rootelement, that.itemChangeFunction, that.itemCreationFunction);
		// models
		$(modelstorage.checkExisting(that.modelAvailable)).bind('modelAvailable', that.modelAvailable);
	}
	
	this.clear = function() {
		if (!that.$rootelement) return;
		that.$rootelement.remove();
		delete that.$rootelement;
		delete that.listview;
	}
	
	this.getName = function(key) {
		var count = that.namemodel.count();
		for(i=0;i<count;++i) {
			var item = that.namemodel.getItem(i);
			if (item.channel == key) return item.name;
		}
		return key;
	}
	
	this.itemChangeFunction = function($domitem, modelitem) {
		var itemText = $domitem.data("itemText");
		var itemSlider = $domitem.data("itemSlider");
		itemText.text(that.getName(modelitem.channel));
		itemSlider.slider("value", modelitem.value);
		return $domitem;
	}
	
	this.itemCreationFunction = function(modelitem) {
		var $item = $('<div class="led"></div>');
		var itemText = $('<div class="ledtext" />');
		var itemSlider = $('<div class="ledslider" />');
		$item.append(itemSlider).data("itemSlider", itemSlider);
		$item.append(itemText).data("itemText", itemText);
		
		itemSlider.slider({min: 0, max: 255, value: 0, orientation: 'horizontal'}).attr("channel", modelitem.channel);
		itemSlider.bind( "slide", function(event, ui) {that.changeled($(this).attr("channel"), ui.value);});
		return $item;
	}

	this.modelAvailable = function(event, modelid, modeldata) {
		if (that.datamodel && that.namemodel) return;
		if (modelid == "led.value") that.datamodel = modeldata;
		else if (modelid == "led.name") that.namemodel = modeldata;
		if (!that.datamodel || !that.namemodel) return;
		that.listview.setModel(that.datamodel);
		
		/*TODO fake daten */
		console.log("add fake data", modelid);
		that.namemodel.reset("channel");
		that.namemodel.change({"channel":"led1","name":"Led 1","id":"led.name"});
		that.datamodel.reset("channel");
		that.datamodel.change({"channel":"led1","value":120,"id":"led.value"});
	}

	this.changeled = function(channel, value) {
		sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"ledvalue_absolut","channel":channel,"value":value});
	}
}
