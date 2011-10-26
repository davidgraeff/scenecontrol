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
				that.connected = true;
				$(that).trigger('onclose');
			}
		} catch(exception) {
			
		}
		console.log(this);
		return true;
	}
}

$(function() {
	propertiesWebsocketInstance = new propertiesWebsocket();
	propertiesWebsocketInstance.reconnect();
});
