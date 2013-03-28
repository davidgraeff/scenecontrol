var storage = require('./storage.js');
var services = require('./services.js');
var properties = require('./properties.js');
var api = require('./com/api.js').api;

exports.waitForAckSceneItem = function(sceneItemUID, scene, runtime, lastresponse) {
	var that = this;
	this.item = sceneItemUID;
	this.service = null;
	this.ackID = sceneItemUID.type_ + "." + sceneItemUID.id_;
	this.timer = null;
	
	this.receivedItem = function(sceneitem) {
		that.item = sceneitem;
		// determine service
		that.service = services.getService(that.item);
		// if no service or scene item is an event, execute successors
		if (sceneItemUID.type_ == "event" || !that.service) {
			that.receiveAck(false); // execute successors. nothing to do here
			return;
		}
		// ack from service to go on with the execution
		that.service.on("ack", that.receiveAck);
		
		// timeout if no answer by the service in time (~15s)
		if (that.item.componentid_!="core")
			that.timer = setTimeout(that.receiveAck, 15000);
		
		// apply property and variable substitution
		for (var propid in that.item) {
			var value = that.item[propid];
			if (typeof value != "string" || !value.length || value[0] != "@")
				continue;
			var propOrVarName = value.slice( 1 ); // remove "@"
			// try lastresponse
			if (propOrVarName=="lastresponse")
				value = lastresponse;
			// try variable
			if (!value)
				value = variables.apply(propOrVarName);
			// try property
			if (!value)
				value = properties.apply(propOrVarName);
			// assign
			if (!value)
				that.item[propid] = "";
			else
				that.item[propid] = value;
		}
		
		// execute item now
		api.setAckRequired(that.item, that.ackID);
		that.service.com.send(that.item);
		//console.log("  WaitAck execute:", that.ackID);
	}
	
	this.execute = function() {
		storage.getSceneItem(sceneItemUID.type_, sceneItemUID.id_, function(err, items) {
			if (err) {
				that.receiveAck(); // execute successors. nothing to do here
				return;
			}
			// we got the scene item from the storage, determine the service now
			if (items && items.length>=0) {
				that.receivedItem(items[0]);
			}
		});
	}
	
	this.abortDestroy = function() {
		if (that.item.sceneid_)
			that.service.abort(that.item.sceneid_);
		that.destroy();
	}
	
	this.destroy = function() {
		if (that.service)
			that.service.removeListener("ack", that.receiveAck);
		if (that.timer)
			clearTimeout(that.timer);
	}
	
	this.receiveAck = function(responseid, response) {
		if (response && responseid != that.ackID)
			return;
		that.destroy();
		
		// get successors
		var successors = [];
		for (var i =0;i<scene.v.length;++i) {
			if (scene.v[i].type_==that.item.type_ && scene.v[i].id_ == that.item.id_) {
				var linked;
				if (that.item.type_ == "condition" && response==false)
					linked = scene.v[i].eAlt;
				else
					linked = scene.v[i].e;
				
				if (linked) {
					for (var j =0;j<linked.length;++j) {
						successors.push({type_:linked[j].type_, id_: linked[j].id_});
					}
				}
			}
		}
		
		if (successors.length)
			runtime.startItemExecution(successors, response, that.item);
	}
}