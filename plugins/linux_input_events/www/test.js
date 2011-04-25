function InitPlugin(pluginid, sectionname, $section) {
	var $panel = $('<span class="ui-widget-header ui-corner-all" />');
	$panel.append($('<button>Vorheriger</button>').button({
			text: false,
			icons: { primary: "ui-icon-seek-start" }
			}).click(function() {
				sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"xbmccmd","state":4});
			})
	);
	$panel.append($('<button>Zurückspulen</button>').button({
			text: false,
			icons: { primary: "ui-icon-seek-prev" }
			}).click(function() {
				sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"xbmcposition","relative":1,"position":-100});
			})
	);
	$panel.append($('<button>Start/Pause</button>').button({
			text: false,
			icons: { primary: "ui-icon-play" }
			}).click(function() {
				sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"xbmccmd","state":0});
			})
	);
	$panel.append($('<button>Stop</button>').button({
			text: false,
			icons: { primary: "ui-icon-stop" }
			}).click(function() {
				sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"xbmccmd","state":2});
			})
	);
	$panel.append($('<button>Vorspulen</button>').button({
			text: false,
			icons: { primary: "ui-icon-seek-next" }
			}).click(function() {
				sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"xbmcposition","relative":1,"position":100});
			})
	);
	$panel.append($('<button>Nächster</button>').button({
			text: false,
			icons: { primary: "ui-icon-seek-end" }
			}).click(function() {
				sessionmanager.socket_write({"__type":"execute","__plugin":pluginid,"id":"xbmccmd","state":3});
			})
	);
	$section.append($panel);
}