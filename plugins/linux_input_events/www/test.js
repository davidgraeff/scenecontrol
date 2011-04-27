function InitPlugin(pluginid, sectionname, $section) {
	$('\
		<div class="inputdevices">Select input device:\
			<form class="linux_input_devices" name="linux_input_devices"><select id="linux_input_devices" name="devices" size="1"><option value="">-- Select device --</option></select>\
			</form>\
		<div id="linux_input_lastkey"></div>\
		</div>\
	').appendTo($section);
	
	function modelAvailable(event, modelid, modeldata) {
		if (!modeldata || (modelid && modelid != "inputdevice")) return;

 		$.getCss(pluginid+"/"+sectionname+".css");

		function itemChangeFunction(domitem, modelitem) {
			return domitem.val(modelitem.device_path).text(modelitem.device_info);
		}
		
		function itemCreationFunction(modelitem) {
			return $("<option/>").val(modelitem.device_path).text(modelitem.device_info).appendTo("#linux_input_devices");
		}

		var $root = $("#linux_input_devices");
		
		var listview = new ListView($root, itemChangeFunction, itemCreationFunction, modeldata);
		
		$root.change(function() {
			var value = $("select option:selected").val();
			if (value.length)
				sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"selected_input_device","device_path":value});
		}).change();

		$(sessionmanager).bind('notification', function(event, data) {
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
		});
	}
	
	$(modelstorage).bind('modelAvailable', modelAvailable);
	modelAvailable(0, 0, modelstorage.getModel("inputdevice"));
}