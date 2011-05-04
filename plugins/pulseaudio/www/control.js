function RoomPlugin(pluginid, sectionname, $section) {
	var that = this;
	this.$rootelement = $('<ul class="pulseaudiochannels"></ul>');
	this.datamodel = undefined;
	this.listview = undefined;
	
	this.load = function() {
		if (!that.$rootelement) return;
		$section.append(that.$rootelement);
		$.getCss(pluginid+"/"+sectionname+".css");
		that.listview = new AbstractView(that.$rootelement, that.itemChangeFunction, that.itemCreationFunction).setModelByName("pulse.channels");
	}
	
	this.clear = function() {
		if (!that.$rootelement) return;
		that.$rootelement.remove();
		delete that.$rootelement;
		delete that.listview;
	}
	
	this.itemChangeFunction = function($domitem, modelitem) {
		$domitem.data("itemText").text(modelitem.sinkid);
		$domitem.data("itemSlider").slider("value", modelitem.volume*10000);
		var options;
		if ( !modelitem.mute ) {
			options = {
				label: "unmuted",
				icons: {
					primary: "ui-icon-volume-on"
				}
			};
		} else {
			options = {
				label: "muted",
				icons: {
					primary: "ui-icon-volume-off"
				}
			};
		}
		$domitem.data("itemMute").button( "option", options );
		
		return $domitem;
	}
	
	this.itemCreationFunction = function(modelitem) {
		var $item = $('<li class="pulsechannel ui-button ui-widget ui-state-default ui-corner-all ui-button-text-only"></li>');
		var itemMute = $('<button class="channelmute" />');
		var itemText = $('<span class="channelname" />');
		var itemSlider = $('<div class="channelvalue" />');
		$item.append(itemMute).data("itemMute", itemMute);
		$item.append(itemText).data("itemText", itemText);
		$item.append(itemSlider).data("itemSlider", itemSlider);
		
		itemMute.button({text: false,icons: {primary: "ui-icon-volume-off"}}).click(function() {
			that.muteChannel(modelitem.sinkid, $( this ).text() != "muted");
		});
		itemSlider.slider({min: 0, max: 10000, value: 0, orientation: 'horizontal'});
		itemSlider.bind( "slide", function(event, ui) {that.changeChannel(modelitem.sinkid, ui.value/10000);});
		return $item;
	}

	this.changeChannel =  function(sindid, volume) {
		sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"pulsechannelvolume","volume":volume,"sindid":sindid});
	}

	this.muteChannel = function(sindid, mute) {
		sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"pulsechannelmute","mute":mute,"sindid":sindid});
	}
}