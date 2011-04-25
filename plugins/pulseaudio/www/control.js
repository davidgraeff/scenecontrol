function InitPlugin(pluginid, sectionname, $section) {
	var $rootelement = $('<ul class="pulseaudiochannels"></ul>');
	$section.append($rootelement);

	function modelAvailable(event, modelid, modeldata) {
		if (!modeldata || (modelid && modelid != "pulse.channels")) return;

 		$.getCss(pluginid+"/"+sectionname+".css");

		function itemChangeFunction(domitem, modelitem) {
			domitem.itemText.text(modelitem.sinkid);
			domitem.itemSlider.slider("value", modelitem.volume*10000);
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
			domitem.itemMute.button( "option", options );
			
			return domitem;
		}
		
		function changeChannel(sindid, volume) {
			sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"pulsechannelvolume","volume":volume,"sindid":sindid});
		}

		function muteChannel(sindid, mute) {
			sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"pulsechannelmute","mute":mute,"sindid":sindid});
		}
		
		function itemCreationFunction(modelitem) {
			var item = $('<li class="pulsechannel ui-button ui-widget ui-state-default ui-corner-all ui-button-text-only"></li>');
			var itemMute = $('<button class="channelmute" />');
			var itemText = $('<span class="channelname" />');
			var itemSlider = $('<div class="channelvalue" />');
			item.itemText = itemText;
			item.itemMute = itemMute;
			item.itemSlider = itemSlider;
			item.append(itemMute);
			item.append(itemText);
			item.append(itemSlider);
			
			itemMute.button({text: false,icons: {primary: "ui-icon-volume-off"}}).click(function() {
				muteChannel(modelitem.sinkid, $( this ).text() != "muted");
			});;
			itemSlider.slider({min: 0, max: 10000, value: 0, orientation: 'horizontal'});
			itemSlider.bind( "slide", function(event, ui) {changeChannel(modelitem.sinkid, ui.value/10000);});
			return item;
		}

		var listview = new ListView($rootelement, itemChangeFunction, itemCreationFunction, modeldata);
	}
	
	$(modelstorage).bind('modelAvailable', modelAvailable);
	modelAvailable(0, 0, modelstorage.getModel("pulse.channels"));
}