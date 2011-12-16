function RoomcontrolPlugin(pluginid, sectionname) {
	var that = this;

	this.currentmode = new Ext.Toolbar({title: 'Aktueller Modus nicht bekannt!'});
	this.newmode = new Ext.form.Text({
		autoComplete: true,
		label: 'Modus',
		placeHolder: 'movie, audio, ...'
	});
	
	this.card = new Ext.form.FormPanel({
		standardSubmit : true,
		defaults: {
			required: true,
			labelAlign: 'left',
			labelWidth: '40%'
		},
		listeners : {
			beforesubmit : function(){
				var value = that.newmode.getValue();
				if (value) {
					that.newmode.setValue('');
					roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":pluginid,"id":"changemode","mode":value});
				} else {
					Ext.Msg.alert('Error', 'Set the new mode!', Ext.emptyFn);
				}
				return false;
			}
		},
		items: [
			that.currentmode,
			that.newmode,
			{
				xtype: 'button',
				margin: 10,
				ui: 'confirm',
				text: 'Modus wechseln',
				handler: function() {
					that.card.submit();
				}
			}
		]
	});
	
	this.init = function() {
		roomcontrol.SessionController.events.on('notification', that.notification, this);
	}
	
	this.clear = function() {
		
	}
	
	this.notification = function(data) {
		if (data.__plugin != pluginid)
			return;

		if (data.id == "mode") {
			this.currentmode.setTitle('Aktueller Modus: ' + data.mode);
		}
	}
}
