function RoomcontrolPlugin(pluginid, sectionname) {
	var that = this;

	this.macs = new Ext.form.Select({
		autoComplete: true,
		label: 'Mac',
		valueField: 'mac',
		displayField: 'ip',
		tpl: '<tpl for="."><strong>{ip}</strong><br/>{mac}</tpl>',
		placeHolder: 'No IPs in arp chache!',
		store: Ext.StoreMgr.lookup("wol.arpcache")
	});
	
	this.card = new Ext.Panel({
		items: [
			that.macs,
			{
				xtype: 'button',
				margin: 10,
				text: 'Rechner aufwecken',
				handler: function() {
					var value = that.macs.getValue();
					if (value)
						roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":pluginid,"id":"wol","mac":value});
				}
			}
		]
	});
	
	this.init = function() {
	}
	
	this.clear = function() {
		
	}
}
