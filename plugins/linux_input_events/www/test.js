function RoomPlugin(pluginid, sectionname, $section) {
	var that = this;
	this.datamodel = undefined;
	this.listview = undefined;
	
	this.load = function() {
		$section.append('\
		<div class="inputdevices">Select input device:\
			<form class="linux_input_devices" name="linux_input_devices"><select id="linux_input_devices" name="devices" size="1"><option value="">-- Select device --</option></select>\
			</form>\
		<div id="linux_input_lastkey"></div>\
		</div>\
		');
		$.getCss(pluginid+"/"+sectionname+".css");
		
		var $root = $("#linux_input_devices").change(function() {
			$('#linux_input_lastkey').empty();
			var udid = $("#linux_input_devices option:selected").val();
			sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"selected_input_device","udid":udid});
		});
		
		that.listview = new AbstractView($root, that.itemChangeFunction, that.itemCreationFunction).setModelByName("inputdevice");
		$(sessionmanager).bind('notification', that.notification);
	}
	
	this.clear = function() {
		delete that.listview;
	}
	
	this.itemChangeFunction = function($domitem, modelitem) {
		return $domitem.val(modelitem.udid).text(modelitem.info);
	}

	this.itemCreationFunction = function(modelitem) {
		return $('<option value="" />').val(modelitem.udid).text(modelitem.info).appendTo("#linux_input_devices");
	}

	this.notification = function(event, data) {
		if (data.__plugin != pluginid)
			return;

		if (data.id == "input.device.key") {
			$('#linux_input_lastkey').text(data.kernelkeyname);
		} else 
		if (data.id == "input.device.selected") {
			if (data.listen)
				$('#linux_input_lastkey').text("Connected, waiting for event...");
			else
				$('#linux_input_lastkey').text("Error: " + data.errormsg);
		}
	}
}
