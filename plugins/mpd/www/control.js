function RoomcontrolPlugin(pluginid, sectionname) {
	var that = this;
	this.status = new Ext.Toolbar();
	
	this.card = new Ext.Panel({
		defaults: {
			xtype: 'button',
			margin: 10
		},
		items: [
			that.status,
			{
				text: 'Vorheriger',
				handler : function() {
					roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":"mpd","id":"mpdcmd","state":4});
				}
			},
			{
				text: 'Zurückspulen',
				handler : function() {
					roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":"mpd","id":"mpdposition","relative":1,"position_in_ms":-5000});
				}
			},
			{
				text: 'Start/Pause',
				handler : function() {
					roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":"mpd","id":"mpdcmd","state":0});
				}
			},
			{
				text: 'Stop',
				ui: 'decline',
				handler : function() {
					roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":"mpd","id":"mpdcmd","state":2});
				}
			},
			{
				text: 'Vorspulen',
				handler : function() {
					roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":"mpd","id":"mpdposition","relative":1,"position_in_ms":5000});
				}
			},
			{
				text: 'Nächster',
				handler : function() {
					roomcontrol.SessionController.writeToServer({"__type":"execute","__plugin":"mpd","id":"mpdcmd","state":3});
				}
			}
		]
	});
	
	this.init = function() {
		$(roomcontrol.SessionManager).bind('notification', that.notification);
	}
	
	this.clear = function() {
		
	}

	this.notification = function(event, data) {
		if (data.__plugin != "mpd")
			return;

		if (data.id == "connection.state") {
			that.status.setTitle((data.state==0)?"No server connected":"Connected");
		} else
		if (data.id == "playlist.current") {
			that.status.setTitle(data.playlistid);
		} else
		if (data.id == "track.info") {
			that.status.setTitle(data.trackname);
		}
	}
}
