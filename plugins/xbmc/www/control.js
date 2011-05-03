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
					sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"xbmccmd","state":4});
				})
		);
		that.$rootelement.append($('<button>Zurückspulen</button>').button({
				text: false,
				icons: { primary: "ui-icon-seek-prev" }
				}).click(function() {
					sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"xbmcposition","relative":1,"position":-100});
				})
		);
		that.$rootelement.append($('<button>Start/Pause</button>').button({
				text: false,
				icons: { primary: "ui-icon-play" }
				}).click(function() {
					sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"xbmccmd","state":0});
				})
		);
		that.$rootelement.append($('<button>Stop</button>').button({
				text: false,
				icons: { primary: "ui-icon-stop" }
				}).click(function() {
					sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"xbmccmd","state":2});
				})
		);
		that.$rootelement.append($('<button>Vorspulen</button>').button({
				text: false,
				icons: { primary: "ui-icon-seek-next" }
				}).click(function() {
					sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"xbmcposition","relative":1,"position":100});
				})
		);
		that.$rootelement.append($('<button>Nächster</button>').button({
				text: false,
				icons: { primary: "ui-icon-seek-end" }
				}).click(function() {
					sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"xbmccmd","state":3});
				})
		);
	}
	
	this.clear = function() {
		if (!that.$rootelement) return;
		that.$rootelement.remove();
		delete that.$rootelement;
	}
}
