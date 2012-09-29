function serverWebsocket() {
	var that = this;
	that.connected = false;
	var socket_di = null;
	that.url;
	
	this.defaultHostAndPort = function() {
		var v = localStorage.getItem("hostAndPort");
		return v ? v : "127.0.0.1:3102";
	}
	
	this.setHostAndPort = function(hostAndPort) {
		localStorage.setItem("hostAndPort", hostAndPort);
		that.url = "ws://"+hostAndPort;
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
		socket_di.send(data+"\n");
	}
	
	this.reconnect = function() {
		if (that.connected || !that.url) return true;
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
					if (datas[index] && datas[index] != '') {
						$(that).trigger('ondocument', JSON.parse(datas[index]));
					}
				}
			} 
			
			socket_di.onclose = function(){
				that.connected = false;
				$(that).trigger('onclose');
			}
		} catch(exception) {
			console.log("websocket exception");
		}
		//console.log(this);
		return true;
	}
}

websocketInstance = new serverWebsocket();