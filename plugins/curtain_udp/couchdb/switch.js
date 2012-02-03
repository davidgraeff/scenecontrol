function RoomcontrolPlugin(pluginid, sectionname) {
	var that = this;
	var store;
	
	this.asciiOnly = function(str)
	{
			var nonASCII=/([^\x00-\x7F])/;
	
			while( str.match(nonASCII) )
			{
					str = str.replace( new RegExp( String(RegExp.$1),"g"),"");
			}
	return str;
	}
	
	this.card = new Ext.form.FormPanel({
		submitOnAction: false,
		defaults: {
			labelWidth: '130px'
		},
		fullscreen: true
	});
	
	this.add = function(store, records, index) {
		for (i=0, l=records.length; i<l; ++i) {
			var data = records[i].data;
			var id = pluginid+this.asciiOnly(records[i].getId());
			var name = (data.channel.length?data.name:'Noname '+index);
			var slider = this.card.getComponent(id);
			if (!slider) {
				slider = [{
						xtype   : 'sliderfield',
						label: name,
						id: id,
						isChanging: false,
						channel: data.channel,
						minValue: 0,
						maxValue: 255,
						value: data.value,
						listeners: {
							drag: function( slider, thumb, newValue ) {
								this.isChanging = true;
								roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":pluginid,"id":"udpled.value_absolut","channel":this.channel,"value":newValue});
							},
							dragend: function() {
								this.isChanging = false;
							},
							el: {
								tap: function(item){
									Ext.Msg.prompt('New name', '', function(buttonid, value) {
										value = escapeInputForJson(value);
										if (buttonid == 'ok' && value.length && name != value) {
											item.target.childNodes[0].textContent = value + '*';
											//console.log("HBAKBF", item.target.childNodes[0].textContent);
											roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":pluginid,"id":"udpled.name","channel":data.channel,"name":value});
										}
									}, null, false, name, {focus: true});
								},
								delegate: '.x-form-label'
							}/*,
							afterrender: function() {
								this.setValue(120);
								console.log("set"+data.value, this.getValue());
							}*/
						}
					}];
				this.card.insert(index, slider);
				//this.card.doLayout();
			} else if (data.value !== undefined && !slider.isChanging) {
				slider.setValue(data.value, 10, true);
			}
			if (data.name) {
				var d = Ext.fly(id);
				if (d && d.dom)
					d.dom.children[0].children[0].textContent = data.name;
			}
				
		}
 		this.card.doLayout();
	}

	this.remove = function(store, record, index) {
		this.card.remove(pluginid+this.asciiOnly(record.getId()), true);
		this.card.doLayout();
	}
	
	this.initAfterServiceLoad = function() {
		that.clear();
		that.store = Ext.StoreMgr.lookup("udpled.names");
		that.store.on('add', that.add, that);
		that.store.on('remove', that.remove, that);
		that.add(that.store, that.store.data.items, 0);	}
	
	this.init = function() {
		this.card.items.clear();
	}
	
	this.clear = function() {
		if (this.store) {
			this.store.un('add', this.add);
			this.store.un('remove', this.remove);
		}
		this.store = undefined;
		this.card.removeAll(true);
	}
}
