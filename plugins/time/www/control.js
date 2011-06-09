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
	
	this.card = new Ext.Panel({
	});
	
	this.elemSetChanged = function(element, changed) {
		var c = element.getComponent(3);
		if (c)
			element.remove(c);
		
		if (changed && !element.isChanged) {
			element.insert(3, {
				xtype: 'button',
				ui  : 'confirm',
				text: 'Apply',
				handler: function() {
					that.elemSetChanged(element, false);
				}
			});
			element.doLayout();
		}
		element.isChanged = changed;
	}
	
	this.add = function(store, records, index) {
		for (i=0, l=records.length; i<l; ++i) {
			var data = records[i].data;
			var id = 'timeevent_'+this.asciiOnly(records[i].getId());
			var element = this.card.getComponent(id);
			
			var event = roomcontrol.SessionController.services[data.uid];
			if (!event)
				continue;
			var collectionNames = roomcontrol.SessionController.getCollectionsWithChildByUid(data.uid);
			
			if (!element) {
				element = new Ext.form.FormPanel({
					id: id,
					isChanged: false,
					submitOnAction: false,
					defaults: {
						labelWidth: '50%'
					}
				});
				this.card.insert(index, element);
			}
			
			element.removeAll();
			element.add([
				{xtype:'selectfield',label:'Profile', options: collectionNames},
				{xtype:'field', inputType:'time',label:'Zeit', value: event.time, init: true,
					listeners: {
						change: function(t, newV, oldV) {
							console.log("CH", t.init);
							if (t.init) {delete t.init; return; }
							event.time = newV;
							that.elemSetChanged(element, true);
						}
					}
				}
			]);
			if (event.id == 'timedate')
				element.add([{xtype:'datepickerfield', label:'Datum', value: event.date, init: true,
					listeners: {
						change: function(t, newV, oldV) {
							if (t.init) {delete t.init; return; }
							event.date = newV;
							that.elemSetChanged(element, true);
						}
					}
				}
			]);
			else if (event.id == 'timeperiodic')
				var weekdays = ["Mo", "Di", "Mi", "Do", "Fr", "Sa", "So"];
				var tage = '';
				for (var wd=0;wd<weekdays.length;++wd) {
					if (event.days.indexOf(wd+"") != -1)
						tage += weekdays[wd] + " ";
				}
				element.add([{xtype:'button', text: 'Tage: ' + tage
					handler: function() {
						var actiondialog;
						var actionsBase = {
							model: true,
							floating: true,
							centered: true,
							width: 300,
							height: 280,
							scroll: 'vertical',
							dockedItems: [{dock: 'top', xtype: 'toolbar', title:'Wochentage'}],
							defaults: {
								xtype: 'checkboxfield',
								margin: 10
							},
							items: [{label: "Montag"}, {label: "Dienstag"}, {label: "Mittwoch"}, {label: "Donnerstag"}, {label: "Freitag"}, {label: "Samstag"}, {label: "Sonntag"},{
								text : 'Ok',
								xtype: 'button',
								ui  : 'confirm',
								scope : this,
								handler : function(){
									// weekday dialog: apply clicked: save days
									event.days = [];
									tage = '';
									for (var wd=0;wd<weekdays.length;++wd) {
										if (actiondialog.getComponent(wd).isChecked()) {
											tage += weekdays[wd] + " ";
											event.days.push(wd);
										}
									}
									that.elemSetChanged(element, true);
									this.setText('Tage: ' + tage);
									actiondialog.hide();
								}
							}]
						};
						if (Ext.is.Phone) {
							actionsBase.fullscreen = true;
							actionsBase.hideOnMaskTap = false;
						}
						
						actiondialog = new Ext.Panel(actionsBase);
						for (var wd=0;wd<weekdays.length;++wd) {
							if (event.days.indexOf(wd+"") != -1)
								actiondialog.getComponent(wd).setChecked(true);
						}
						actiondialog.show();
					}
				}]);
		}
		element.add({xtype:'panel',html:''});
		element.doLayout();
		this.card.doLayout();
	}

	this.remove = function(store, record, index) {
		this.card.remove('timeevent_'+this.asciiOnly(record.getId()), true);
		this.card.doLayout();
	}

	this.init = function() {
	}
	
	this.initAfterServiceLoad = function() {
		this.clear();
		this.store = Ext.StoreMgr.lookup("time.alarms");
		this.store.on('add', this.add, this);
		this.store.on('remove', this.remove, this);
		this.add(this.store, this.store.data.items, 0);
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
