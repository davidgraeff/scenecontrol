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
				text: 'Vorheriger',
				handler : function() {
					roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":pluginid,"id":"xbmccmd","state":4});
				}
			},
			{
				text: 'Zurückspulen',
				handler : function() {
					roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":pluginid,"id":"xbmcposition","relative":1,"position":-100});
				}
			},
			{
				text: 'Start/Pause',
				handler : function() {
					roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":pluginid,"id":"xbmccmd","state":0});
				}
			},
			{
				text: 'Stop',
				ui: 'decline',
				handler : function() {
					roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":pluginid,"id":"xbmccmd","state":2});
				}
			},
			{
				text: 'Vorspulen',
				handler : function() {
					roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":pluginid,"id":"xbmcposition","relative":1,"position":100});
				}
			},
			{
				text: 'Nächster',
				handler : function() {
					roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":pluginid,"id":"xbmccmd","state":3});
				}
			}
		]
	});
	
	this.init = function() {
	}
	
	this.clear = function() {
		
	}
}
