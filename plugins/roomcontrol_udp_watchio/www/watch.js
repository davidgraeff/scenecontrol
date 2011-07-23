function RoomcontrolPlugin(pluginid, sectionname) {
	var that = this;
	var store;
	
	this.card = new Ext.List({
		itemTpl : 'Pin: {sensorid} <small>aktiv: {value}</small>',
		grouped : false,
		indexBar: false,
		singleSelect: false,
		store: Ext.StoreMgr.lookup("udpwatchio.sensor")
	});
	
	this.init = function() {
		that.store = Ext.StoreMgr.lookup("udpwatchio.sensor");
		console.log(that.store);
	}
	
	this.clear = function() {
	}
}
