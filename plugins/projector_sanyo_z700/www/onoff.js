function RoomcontrolPlugin(pluginid, sectionname) {
	var that = this;
	this.status = new Ext.Toolbar();
	
	this.card = new Ext.Panel({
		defaults: {
			xtype: 'button',
			margin: 10
		},
		items: [
			{
				text: 'Einschalten',
				ui: 'confirm',
				handler : function() {
					roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":pluginid,"id":"projector_sanyo_power","power":1});
				}
			},
			{
				text: 'Ausschalten',
				ui: 'decline',
				handler : function() {
					roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":pluginid,"id":"projector_sanyo_power","power":0});
				}
			},
			{
				text: 'Schwarzes Bild',
				handler : function() {
					roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":pluginid,"id":"projector_sanyo_video","mute":1});
				}
			},
			{
				text: 'Bild wiederherstellen',
				handler : function() {
					roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":pluginid,"id":"projector_sanyo_video","mute":0});
				}
			}
		]
	});
	
	this.init = function() {
	}
	
	this.clear = function() {
		
	}
}
