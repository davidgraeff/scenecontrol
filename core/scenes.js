var configs = require('./config.js');
var storage = require('./storage.js');
var services = require('./services.js');
var api = require('./com/api.js').api;
var assert = require('assert');

var scenes = {};

function waitForAckSceneItem(sceneItemUID, scene, runtime, lastresponse) {
	var that = this;
	this.item = sceneItemUID;
	this.service = null;
	this.ackID = sceneItemUID.type_ + "." + sceneItemUID.id_;
	this.timer = null;
	
	this.execute = function() {
		storage.getSceneItem(sceneItemUID.type_, sceneItemUID.id_, function(err, items) {
			if (err) {
				that.receiveAck(); // execute successors. nothing to do here
				return;
			}
			// we got the scene item from the storage, determine the service now
			if (items && items.length>=0) {
				that.item = items[0];
				// determine service
				that.service = services.getService(that.item);
				// if no service or scene item is an event, execute successors
				if (sceneItemUID.type_ == "event" || !that.service) {
					that.receiveAck(false); // execute successors. nothing to do here
					return;
				}
				// ack from service to go on with the execution
				that.service.on("ack", that.receiveAck);
				
				// timeout if no answer by the service in time (~1,5s)
				that.timer = setTimeout(that.receiveAck, 1500);

				// TODO apply properties+variables now (incl. lastresponse)
				
				// execute item now
				api.setAckRequired(that.item, that.ackID);
				that.service.com.send(that.item);
				console.log("  WaitAck execute:", that.ackID);
			}
			
		});
	}
	
	this.destroy = function() {
		that.service.removeListener("ack", that.receiveAck);
		if (that.timer)
			clearTimeout(that.timer);
	}
	
	this.receiveAck = function(responseid, response) {
		if (response && responseid != that.ackID)
			return;
		clearTimeout(that.timer);
		
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
		
		runtime.startItemExecution(successors, response, that.item);
	}
}

function sceneRuntime(sceneDoc) {
	var that = this;
	this.scene = sceneDoc;
	// scene-event handling
	this.eventsBeforeReload = {};
	this.eventsreloadCounter = 0;
	this.eventsToBeRegistered = {};
	// scene-runtime handling
	this.waitForAckSceneItems = [];
	
	services.services.on("added", function(service) {
		// copy object
		var eventids = [];
		for (var eventid in that.eventsToBeRegistered)
			eventids.push(eventid);
		
		eventids.forEach(function(eventid) {
			var event = that.eventsToBeRegistered[eventid];
			if (services.documentIsForService(event, service)) {
				that.registerEvent(event);
				delete that.eventsToBeRegistered[eventid];
			}
		});
	});
	
	this.reload = function(sceneDoc) {
		this.sceneDoc = sceneDoc;
		storage.getEventsForScene(sceneDoc.id_, function(err, eventDocs) {
			if (err) {
				console.warn("Could not get scenes ", err);
			}
			if (eventDocs) {
				that.eventsToBeRegistered = {};
				// set counter for this reload iteration
				++that.eventsreloadCounter;
				console.log("Scene: ", sceneDoc.name);
				// register events on services
				eventDocs.forEach(function(eventDoc) {
					if (that.eventsBeforeReload[eventDoc.id_]) {
						that.unregisterEvent(eventDoc);
					}
					that.registerEvent(eventDoc);
					that.eventsBeforeReload[eventDoc.id_].changeIteration = that.eventsreloadCounter;
				});
				// unregister not used events
				for (var eventid in that.eventsBeforeReload) {
					if (that.eventsBeforeReload[eventid].changeIteration != that.eventsreloadCounter) {
						that.unregisterEvent(eventid);
					}
				}
			}
			
		});
	}
	
	/**
	 * This method will add all listed nodes to the waitForAckSceneItems list and execute them.
	 */
	this.startItemExecution = function(sceneItemUIDs, lastresponse, removeOldUID) {
		console.log("Start scene items @ ", that.scene.name, sceneItemUIDs.length);
		sceneItemUIDs.forEach(function(sceneItemID) {
			var ackObj = new waitForAckSceneItem(sceneItemID, that.scene, that, lastresponse);
			that.waitForAckSceneItems.push(ackObj);
			ackObj.execute();
		});
		if (removeOldUID) {
			that.waitForAckSceneItems = that.waitForAckSceneItems.filter(function(elem) {return elem.item != removeOldUID});
		}
	}
	
	/**
	 * Some scene items do not have "incoming" other scene items.
	 * Those are called root nodes. This method returns all root nodes
	 * of the current scene.
	 */
	this.determineRootNodes = function() {
		var linkedNodes = [];
		var allNodes = [];
		var rootNodes = [];
		for (var id in that.scene.v) {
			var sceneitem = that.scene.v[id];
			if (sceneitem.type_==undefined || sceneitem.id_==undefined)
				continue;
			var e = sceneitem.e;
			sceneitem = {type_:sceneitem.type_, id_:sceneitem.id_};
			
			allNodes.push(sceneitem);
			for (var e_id in e) {
				var linkedsceneitem = e[e_id];
				if (linkedsceneitem.type_==undefined || linkedsceneitem.id_==undefined)
					continue;
				linkedsceneitem = {type_:linkedsceneitem.type_, id_:linkedsceneitem.id_};
				if (linkedNodes.indexOf(linkedsceneitem)==-1)
					linkedNodes.push(linkedsceneitem);
			}
		}
		// remove from allnodes if in linkedNodes
		for (var i =0;i<allNodes.length;++i) {
			var found = false;
			for (var j =0;j<linkedNodes.length;++j) {
				if (allNodes[i].type_==linkedNodes[j].type_ && allNodes[i].id_==linkedNodes[j].id_ ) {
					found = true;
					break;
				}
			}
			if (!found)
				rootNodes.push(allNodes[i]);
		}
		return rootNodes;
	}
	
	this.stopScene = function() {
		while (that.waitForAckSceneItems.length) {
			var ackObject = that.waitForAckSceneItems.pop();
			ackObject.destroy();
			delete ackObject;
		}
	}
	
	/**
	 * Start scene either with all root nodes if @sceneItemUIDs is omitted
	 * or with all referenced scene items of @sceneItemUIDs
	 */
	this.startScene = function(sceneItemUIDs) {
		if (!sceneItemUIDs)
			sceneItemUIDs = that.determineRootNodes();
		if (!sceneItemUIDs) // Empty scene? Abort
			return;
		
		that.startItemExecution(sceneItemUIDs);
	}
	
	this.eventTriggered = function(eventDoc) {
		if (eventDoc.sceneid_ != that.scene.id_)
			return;
		
		that.startScene([eventDoc.id_]);
	}
	
	this.registerEvent= function(eventDoc) {
		assert(eventDoc, "registerEvent");
		var service = services.getService(eventDoc);
		that.eventsBeforeReload[eventDoc.id_] = {service: service, event: eventDoc};
		
		if (!service) {
// 			console.log("  Event Later: ", eventDoc.method_);
			that.eventsToBeRegistered[eventDoc.id_] = eventDoc;
		} else {
// 			console.log("  Event: ", eventDoc.method_, service.id);
			service.com.send(eventDoc);
			service.on("event_triggered", that.eventTriggered);
		}
	}
	
	this.unregisterEvent= function(eventid) {
		// unregister -> this event do not have to be registered later anymore
		delete that.eventsToBeRegistered[eventid];
		// unregister means an event document does not have a scene id
		
		var entry = that.eventsBeforeReload[eventid];
		var service = entry.service;
		if (service) {
			service.removeListener("event_triggered", that.eventTriggered);
			delete entry.event.sceneid_;
			service.com.send(entry.event);
		}
		
		delete that.eventsBeforeReload[eventid];
	}

	this.unload = function() {
		that.stopScene();
		//console.log("Unload scene: ", sceneDoc.name);
	}
	
	this.reload(sceneDoc);
}

exports.getScene = function(id) {
	return scenes[id];
}

exports.finish = function(callback) {
	for (var i in scenes) {
		scenes[i].unload();
	}
	callback();
}

exports.init = function(callback) {
	
	function reloadScene(sceneDoc) {
		if (!scenes[sceneDoc.id_])
			scenes[sceneDoc.id_] = new sceneRuntime(sceneDoc);
		else
			scenes[sceneDoc.id_].reload(sceneDoc);
	}
	
	console.log('Starting scenes...');
	storage.getScenes(function(err, items) {
		if (err) {
			console.warn("Could not get scenes ", err);
		}
		if (items) {
			items.forEach(reloadScene);
		}
		
		callback(null, null);
	});
}