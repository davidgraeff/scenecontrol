/**
 * Websocket Module - Communication with the SceneServer via the websocket technology
 * Available as window.websocketInstance.
 */
(function (window) {
	"use strict";
	
	function serverWebsocket() {
		var that = this;
		that.connected = false;
		that.socket_di = null;
		that.url;
		this.state = 1;
		
		this.defaultHostAndPort = function() {
			return  "127.0.0.1:3102";
		}
		
		this.setHostAndPort = function(hostAndPort, useWSS) {
			that.url = ((useWSS)?"wss://":"ws://")+hostAndPort;
		}

		this.startMonitorScene = function(sceneid) {
			this.write({"type_":"execute", "method_":"startMonitor", "monitor": "scene", "sceneid_": sceneid});
		}
		

		this.runcollection = function(sceneid) {
			this.write({"type_":"execute", "method_":"runcollection", "sceneid_": sceneid});
		}
		
		this.write = function(data) {
			if (typeof data == "object")
				data = JSON.stringify(data);
			that.socket_di.send(data+"\n");
		}
		
		this.checkConnected = function() {
			if (that.connected == false) {
				if (that.socket_di)
					that.socket_di.close();
				$(that).trigger('onerror');
			}
		}
		
		this.reconnect = function() {
			if (that.connected || !that.url) return true;
			if (that.socket_di) {
				that.socket_di.close();
			}
			
			try {
				window.setTimeout( that.checkConnected, 1500 );
				that.socket_di = new WebSocket(that.url, "scenecontrol-protocol");
				that.socket_di.onopen = function() {
					that.connected = true;
					$(that).trigger('onopen');
				} 
				
				that.socket_di.onmessage =function(msg) {
					var datas = msg.data.split("\n");
					for (var index in datas) {
						if (datas[index] && datas[index] != '') {

							var doc = JSON.parse(datas[index]);
							if (api.needAck(doc)) {
								that.write(api.generateAck(doc));
							}
							switch (that.state) {
								case 1:
									that.serverinfo = doc;
									that.write(api.methodIdentify("htmleditorv2"));
									that.state = 2;
									break;
								case 2:
									if (!api.isAck(doc, "htmleditorv2")) {
										that.socket.close();
										return;
									}
									that.state = 3;
									$(that).trigger('onidentified');
									break;
								case 3:
									$(that).trigger('ondocument', doc);
							}
						}
					}
				} 
				
				that.socket_di.onclose = function(){
					var v = that.connected;
					that.connected = false;
					if (v==true)
						$(that).trigger('onclose');
				}
			} catch(exception) {
				var v = that.connected;
				that.connected = false;
				if (v==true)
					$(that).trigger('onclose');
				
				$(that).trigger('onerror');
			}
			return true;
		}
	}
	window.websocketInstance = new serverWebsocket();
})(window);
