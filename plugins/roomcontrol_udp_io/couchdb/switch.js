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
			labelWidth: '50%'
		}
	});
	
	this.add = function(store, records, index) {
		for (i=0, l=records.length; i<l; ++i) {
			var data = records[i].data;
			var id = pluginid+this.asciiOnly(records[i].getId());
			var slider = this.card.getComponent(id);
			if (slider) {
				if (data.value)
					slider.setValue(data.value);
				if (data.name)
					Ext.fly(id).dom.children[0].children[0].textContent = data.name;
					
			} else {
				var name = (data.channel.length?data.name:'Noname '+index);
				var element = new Ext.form.Toggle({
					label: name,
					id: id,
					value: data.value,
					isInit: true,
					minValue: 0,
					maxValue: 1,
					listeners: {
						change: function( slider, thumb, newValue, oldValue ) {
							if (slider.isInit) {
								slider.isInit = false;
								return true;
							}
							if (newValue != oldValue) {
								roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":pluginid,"id":"udpio.value_absolut","channel":data.channel,"value":(newValue?true:false)});
							}
						},
						el: {
							tap: function(item){
								Ext.Msg.prompt('New name', '', function(buttonid, value) {
									value = escapeInputForJson(value);
									if (buttonid == 'ok' && value.length && name != value) {
										item.target.childNodes[0].textContent = value + '*';
										//console.log("HBAKBF", item.target.childNodes[0].textContent);
										roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":pluginid,"id":"udpio.name","channel":data.channel,"name":value});
									}
								}, null, false, name, {focus: true});
							},
							delegate: '.x-form-label'
						}
					}
				});
				
				this.card.insert(index, element);
			}
		}
		this.card.doLayout();
	}

	this.remove = function(store, record, index) {
		this.card.remove(pluginid+this.asciiOnly(record.getId()), true);
		this.card.doLayout();
	}

	this.init = function() {
		this.card.items.clear();
		this.store = Ext.StoreMgr.lookup("udpio.names");
		this.store.on('add', this.add, this);
		this.store.on('remove', this.remove, this);
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
