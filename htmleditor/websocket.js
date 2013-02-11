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
		
		this.defaultHostAndPort = function() {
			var v = localStorage.getItem("hostAndPort");
			return v ? v : "127.0.0.1:3102";
		}
		
		this.setHostAndPort = function(hostAndPort, useWSS) {
			localStorage.setItem("hostAndPort", hostAndPort);
			that.url = ((useWSS)?"wss://":"ws://")+hostAndPort;
		}
		this.requestAllDocuments = function() {
			this.write({"componentid_":"server", "type_":"execute", "method_":"fetchAllDocuments"});
		}
		
		this.registerNotifier = function() {
			this.write({"componentid_":"server", "type_":"execute", "method_":"registerNotifier"});
		}
		
		this.requestAllProperties = function() {
			this.write({"componentid_":"server", "type_":"execute", "method_":"requestAllProperties"});
		}
		
		this.version = function() {
			this.write({"componentid_":"server", "type_":"execute", "method_":"version"});
		}
		
		this.runcollection = function() {
			this.write({"componentid_":"server", "type_":"execute", "method_":"runcollection"});
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
				that.socket_di = new WebSocket(that.url, "roomcontrol-protocol");
				that.socket_di.onopen = function() {
					console.log("conn2", that.connected);
					that.connected = true;
					$(that).trigger('onopen');
				} 
				
				that.socket_di.onmessage =function(msg) {
					var datas = msg.data.split("\n");
					for (var index in datas) {
						if (datas[index] && datas[index] != '') {
							$(that).trigger('ondocument', JSON.parse(datas[index]));
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
