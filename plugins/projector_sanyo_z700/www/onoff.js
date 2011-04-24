function InitPlugin(pluginid, sectionname, $section) {
	function activate(value) {
		sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"projector_sanyo_power","power":value});
		return false;
	}
	function mute(value) {
		sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"projector_sanyo_video","mute":value});
		return false;
	}
	
	var $formelement = $('<form class="sanyoZ700" />');
	var $btnEnable = $('<button>Einschalten</button>');
	var $btnDisable = $('<button>Ausschalten</button>');
	var $formelement2 = $('<form class="sanyoZ700" />');
	var $btnVideoMute = $('<button>Schwarzes Bild</button>');
	var $btnVideoUnmute = $('<button>Bild wiederherstellen</button>');
	$btnEnable.button(); $btnDisable.button(); $btnVideoMute.button(); $btnVideoUnmute.button();
	
	$btnEnable.click( function() { return activate(1); });
	$btnDisable.click( function() { return activate(0); });
	$btnVideoMute.click( function() { return mute(1); });
	$btnVideoUnmute.click( function() { return mute(0); });

	$formelement.append($btnEnable);
	$formelement.append($btnDisable);
	$formelement2.append($btnVideoMute);
	$formelement2.append($btnVideoUnmute);

	$section.append($formelement);
	$section.append($formelement2);
}