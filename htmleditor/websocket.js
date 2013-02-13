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
		
		this.createScene = function(name) {
			var newscene = {"id_":"GENERATEGUID", "componentid_":"server", "type_": "scene","v":[],"categories": [],"enabled": true,"name": name};
			websocketInstance.write(newscene);
		}
		
		this.createSceneItem = function(sceneDocument, sceneItemDocument) {
			websocketInstance.write({"componentid_":"server","type_":"execute","method_":"addSceneItemDocument","scene":sceneDocument,"sceneitem":sceneItemDocument});
		}
		
		this.removeSceneItem = function(sceneDocument, sceneItemDocument) {
			websocketInstance.write({"componentid_":"server","type_":"execute","method_":"removeSceneItemDocument","scene":sceneDocument,"sceneitem":sceneItemDocument});
		}
		
		this.createConfig = function(instanceid, componentid) {ee
			var newconfig = {"id_":"GENERATEGUID", "componentid_":componentid, "type_": "configuration","instanceid_": instanceid};
			websocketInstance.write(newconfig);
		}
		
		this.remove = function(sceneDocument) {
			websocketInstance.write({"componentid_":"server","type_":"execute","method_":"removeDocument","doc":sceneDocument});
		}
		
		this.updateDocument = function(sceneDocument) {
			//console.log("Change: ", sceneDocument)
			websocketInstance.write({"componentid_":"server","type_":"execute","method_":"changeDocument","doc":sceneDocument});
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
