function InitPlugin(pluginid, sectionname, $section) {
	$('\
		<div class="inputdevices">Select input device:\
			<form class="linux_input_devices" name="linux_input_devices"><select id="linux_input_devices" name="devices" size="1"></select>\
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
			sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"selected_input_device","device_path":value});
		}).change();

		$(sessionmanager).bind('notification', function(event, data) {
			if (data.__plugin != pluginid)
				return;

			console.log("linux_input_lastkey", data);
			if (data.id == "lastinputkey") {
				$('#linux_input_lastkey').text(data.kernelkeyname);
			}
		});
	}
	
	$(modelstorage).bind('modelAvailable', modelAvailable);
	modelAvailable(0, 0, modelstorage.getModel("inputdevice"));
}