var configs = require('./config.js');
var storage = require('./storage.js');
var services = require('./services.js');
var api = require('./com/api.js').api;
var assert = require('assert');

var scenes = {};

function sceneRuntime(sceneDoc) {
	var that = this;
	this.eventsBeforeReload = {};
	this.eventsreloadCounter = 0;
	this.eventsToBeRegistered = {};
	
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
		storage.getEventsForScene(sceneDoc.id_, function(err, items) {
			if (err) {
				console.warn("Could not get scenes ", err);
			}
			if (items) {
				that.checkEvents(items);
			}
			
		});
	}
	
	this.registerEvent= function(eventDoc) {
		assert(eventDoc, "registerEvent");
		var service = services.getService(eventDoc.componentid_, eventDoc.instanceid_);
		that.eventsBeforeReload[eventDoc.id_] = {service: service, event: eventDoc};
		
		if (!service) {
			console.log("  Event Later: ", eventDoc.method_);
			that.eventsToBeRegistered[eventDoc.id_] = eventDoc;
		} else {
			console.log("  Event: ", eventDoc.method_, service.id);
			service.com.send(eventDoc);
		}
	}
	
	this.unregisterEvent= function(eventid) {
		// unregister -> this event do not have to be registered later anymore
		delete that.eventsToBeRegistered[eventid];
		// unregister means an event document does not have a scene id
		
		var entry = that.eventsBeforeReload[eventid];
		var service = entry.service;
		if (service) {
			delete entry.event.sceneid_;
			service.com.send(entry.event);
		}
		
		delete that.eventsBeforeReload[eventid];
	}
	
	this.checkEvents = function(eventDocs) {
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
	
	this.unload = function() {
		console.log("Unload scene: ", sceneDoc.name);
	}
	
	this.reload(sceneDoc);
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