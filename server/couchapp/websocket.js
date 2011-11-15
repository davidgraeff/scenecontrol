function propertiesWebsocket() {
	var that = this;
	that.connected = false;
	var socket_di = null;
	that.url = "wss://" + window.location.hostname +":3101";
	
	this.requestall = function() {
		this.write({"plugin_":"PropertyController", "type_":"execute", "member_":"requestProperties"});
	}
	
	this.write = function(data) {
		if (typeof data == "object")
			data = JSON.stringify(data);
		socket_di.send(data);
	}
	
	this.reconnect = function() {
		if (window.location.hostname == "") {
			return false;
		}
		
		if (that.connected) return true;
		if (socket_di) socket_di.close();
		
		socket_di = new WebSocket(that.url, "roomcontrol-protocol");
		try {
			socket_di.onopen = function() {
				that.connected = true;
				$(that).trigger('onopen');
			} 
			
			socket_di.onmessage =function(msg) {
				var datas = msg.data.split("\n");
				for (index in datas) {
					if (datas[index] && datas[index] != '')
						$(that).trigger('onproperty', datas[index]);
				}
			} 
			
			socket_di.onclose = function(){
				that.connected = false;
				$(that).trigger('onclose');
			}
		} catch(exception) {
			
		}
		console.log(this);
		return true;
	}
}

function escapeInputForJson(string)
{

	var _escapeable = /["\\\x00-\x1f\x7f-\x9f]/g;

	var _meta = {
		'\b': '\\b',
		'\t': '\\t',
		'\n': '\\n',
		'\f': '\\f',
		'\r': '\\r',
		'"' : '\\"',
		'\\': '\\\\'
	};

	if (string.match(_escapeable))
	{
		return string.replace(_escapeable, function (a) 
		{
			var c = _meta[a];
			if (typeof c === 'string') return c;
			c = a.charCodeAt();
			return '\\u00' + Math.floor(c / 16).toString(16) + (c % 16).toString(16);
		});
	}
	return string;
};

//$.mobile.pushStateEnabled = false;
$db = $.couch.db("roomcontrol");
propertiesWebsocketInstance = new propertiesWebsocket();

$(function() {
	propertiesWebsocketInstance.reconnect();
});
