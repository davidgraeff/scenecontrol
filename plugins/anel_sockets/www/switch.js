function RoomPlugin(pluginid, sectionname, $section) {
	var that = this;
	this.$rootelement = $('<ul class="anelsockets"></ul>');
	this.datamodel;
	this.namemodel;
	this.listview;
	
	this.load = function() {
		if (!that.$rootelement) return;
		$section.append(that.$rootelement);
		$.getCss(pluginid+"/"+sectionname+".css");
		that.listview = new AbstractView(that.$rootelement, that.itemChangeFunction, that.itemCreationFunction);
		$(modelstorage.checkExisting(that.modelAvailable)).bind('modelAvailable', that.modelAvailable);
	}
	
	this.clear = function() {
		that.$rootelement.remove();
		$(that).unbind();
		delete that.listview;
		delete that.datamodel;
		delete that.namemodel;
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
		if (modelitem.value)
			$domitem.removeClass("anelsockets_deactivated").addClass("anelsockets_activated");
		else
			$domitem.removeClass("anelsockets_activated").addClass("anelsockets_deactivated");
		return $domitem.text(that.getName(modelitem.channel));
	}
	
	this.itemCreationFunction = function(modelitem) {
		var $item = $('<li class="anelsockets" contentEditable="true"></li>').attr("channel", modelitem.channel);
		$item.click( function(event) {if(event.shiftKey) {return;} that.tooglevalue($(this).attr("channel")); });
		$item.keydown (filterNamesFunction);
		$item.keyup (function() {
			var newname = $(this).text();
			if (newname.length && newname != that.getName(modelitem.channel)) {
				sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"ioname","channel":modelitem.channel,"name":newname});
			}
		});
		return $item;
	}

	this.removeDataModel = function() {
		that.datamodel = 0;
	}
	this.removeNameModel = function() {
		that.namemodel = 0;
	}
	
	this.modelAvailable = function(event, modelid, modeldata) {
		if (that.datamodel && that.namemodel) return;
		if (modelid == "anel.io.value") {
			that.datamodel = modeldata;
			$(that.datamodel).bind('modelremove', that.removeDataModel);
		}
		else if (modelid == "anel.io.name") {
			that.namemodel = modeldata;
			$(that.namemodel).bind('modelremove', that.removeNameModel);
		}
		if (!that.datamodel || !that.namemodel) return;
		that.listview.setModel(that.datamodel);
	}

	this.tooglevalue = function(key) {
		sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"iovalue_toogle","channel":key});
	}	
}
