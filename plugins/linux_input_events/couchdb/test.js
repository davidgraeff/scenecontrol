function RoomcontrolPlugin(pluginid, sectionname) {
	var that = this;

	this.devices = new Ext.form.Select({
		label: 'Device',
		valueField: 'udid',
		displayField: 'info',
		placeHolder: 'No device found!',
		store: Ext.StoreMgr.lookup("inputdevice"),
		listeners: {
			change: function(t, value) {
				roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":pluginid,"id":"selected_input_device","udid":value});
			}
		}
	});
	
	this.linux_input_lastkey = new Ext.Panel({html:" "});
	
	this.card = new Ext.Panel({
		items: [
			that.devices,
			that.linux_input_lastkey
		],
		listeners: {
			beforehide: function() {
				roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":pluginid,"id":"selected_input_device","udid":""});
			},
			beforeshow: function() {
				var value = that.devices.getValue();
				if (value)
					roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":pluginid,"id":"selected_input_device","udid":value});

			}
		}
	});

	this.init = function() {
		roomcontrol.SessionController.events.on('notification', that.notification, this);
	}
	
	this.clear = function() {
		
	}
	
	this.notification = function(data) {
		if (data.__plugin != pluginid)
			return;

		if (data.id == "input.device.key") {
			that.linux_input_lastkey.update(data.kernelkeyname);
		} else 
		if (data.id == "input.device.selected") {
			if (data.listen)
				that.linux_input_lastkey.update("Connected, waiting for event...");
			else
				that.linux_input_lastkey.update("Error: " + data.errormsg);
		}
	}
}

