function RoomPlugin(pluginid, sectionname, $section) {
	var that = this;
	this.$rootelement = $('<div class="ui-widget-header ui-corner-all" style="width:80%;margin:auto;margin-bottom:1em" />');
	
	this.load = function() {
		if (!that.$rootelement) return;
		$section.append(that.$rootelement).append('<div class="ui-widget-header ui-corner-all" style="width:80%;margin:auto"><span id="mpdconnection"></span><span id="mpdcurrent"></span><span id="mpdtrack"></span></div>');
		that.$rootelement.append($('<button>Vorheriger</button>').button({
				text: false,
				icons: { primary: "ui-icon-seek-start" }
				}).click(function() {
					sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"mpdcmd","state":4});
				})
		);
		that.$rootelement.append($('<button>Zurückspulen</button>').button({
				text: false,
				icons: { primary: "ui-icon-seek-prev" }
				}).click(function() {
					sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"mpdposition","relative":1,"position_in_ms":-5000});
				})
		);
		that.$rootelement.append($('<button>Start/Pause</button>').button({
				text: false,
				icons: { primary: "ui-icon-play" }
				}).click(function() {
					sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"mpdcmd","state":0});
				})
		);
		that.$rootelement.append($('<button>Stop</button>').button({
				text: false,
				icons: { primary: "ui-icon-stop" }
				}).click(function() {
					sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"mpdcmd","state":2});
				})
		);
		that.$rootelement.append($('<button>Vorspulen</button>').button({
				text: false,
				icons: { primary: "ui-icon-seek-next" }
				}).click(function() {
					sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"mpdposition","relative":1,"position_in_ms":5000});
				})
		);
		that.$rootelement.append($('<button>Nächster</button>').button({
				text: false,
				icons: { primary: "ui-icon-seek-end" }
				}).click(function() {
					sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"mpdcmd","state":3});
				})
		);
		
		$(sessionmanager).bind('notification', that.notification);
	}
	
	this.clear = function() {
		that.$rootelement.remove();
		delete that.$rootelement;
	}
	
	this.notification = function(event, data) {
		if (data.__plugin != pluginid)
			return;

		if (data.id == "connection.state") {
			$('#mpdconnection').text((data.state==0)?"No server connected":"Connected");
		} else
		if (data.id == "playlist.current") {
			$('#mpdcurrent').text(data.playlistid);
		} else
		if (data.id == "track.info") {
			$('#mpdtrack').text(data.trackname);
		}
	}

}
