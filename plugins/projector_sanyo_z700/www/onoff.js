function RoomPlugin(pluginid, sectionname, $section) {
	var that = this;
	this.$formelement = $('<form class="sanyoZ700" />');
	this.$btnEnable = $('<button>Einschalten</button>').button().click( function() { return that.activate(1); });
	this.$btnDisable = $('<button>Ausschalten</button>').button().click( function() { return that.activate(0); });
	this.$btnVideoMute = $('<button>Schwarzes Bild</button>').button().click( function() { return that.mute(1); });
	this.$btnVideoUnmute = $('<button>Bild wiederherstellen</button>').button().click( function() { return that.mute(0); });
	
	this.load = function() {
		that.$formelement.append(that.$btnEnable).append('<br/>').append(that.$btnDisable).append('<br/>').append(that.$btnVideoMute).append('<br/>').append(that.$btnVideoUnmute).appendTo($section);
	}
	
	this.clear = function() {
		that.$formelement.remove();
	}
	
	this.activate = function(value) {
		sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"projector_sanyo_power","power":value});
		return false;
	}
	
	this.mute = function(value) {
		sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"projector_sanyo_video","mute":value});
		return false;
	}
}
