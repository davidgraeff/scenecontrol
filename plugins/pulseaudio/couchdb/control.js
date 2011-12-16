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
	
	this.labelname = function(sinkid, mute, changed) {
		return sinkid + ' (' + (mute ? "Muted" : "Unmuted") + (changed?"*":'') + ')';
	}
	
	this.add = function(store, records, index) {
		for (i=0, l=records.length; i<l; ++i) {
			var data = records[i].data;
			var id = 'pulseaudio_'+this.asciiOnly(records[i].getId());
			var element = this.card.getComponent(id);
			if (element) {
				element.mute = data.mute;
				element.isInit = true;
				element.setValue(data.volume*100);
				Ext.fly(id).dom.children[0].children[0].textContent = that.labelname(data.sinkid, data.mute, false);
			} else {
				element = new Ext.form.Slider({
					label: that.labelname(data.sinkid, data.mute, false),
					sinkid: data.sinkid,
					mute: false,
					id: id,
					value: data.volume*100,
					isInit: true,
					minValue: 0,
					maxValue: 100,
					listeners: {
						change: function( slider, thumb, newValue, oldValue ) {
							if (slider.isInit) {
								slider.isInit = false;
								return true;
							}
							if (newValue != oldValue) {
								roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":pluginid,"id":"pulsechannelvolume","volume":newValue/100,"sinkid":slider.sinkid});
							}
						},
						el: {
							tap: function(item){
								element.mute = !element.mute;
								item.target.childNodes[0].textContent = that.labelname(element.sinkid, element.mute, true);
								roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":pluginid,"id":"pulsechannelmute","mute":element.mute,"sinkid":element.sinkid});
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
		this.card.remove('pulseaudio_'+this.asciiOnly(record.getId()), true);
		this.card.doLayout();
	}

	this.init = function() {
		this.card.items.clear();
		this.store = Ext.StoreMgr.lookup("pulse.channels");
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
