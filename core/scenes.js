var configs = require('./config.js');
var storage = require('./storage.js');

var scenes = {};

function sceneRuntime(sceneDoc) {
	this.reload = function(sceneDoc) {
		this.sceneDoc = sceneDoc;
		console.log("Scene: ", sceneDoc.name);
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
	storage.db.collection('scene').find().toArray(function(err, items) {
		if (err) {
			console.warn("Could not get scenes ", err);
		}
		if (items) {
			items.forEach(reloadScene);
		}
		
		callback(null, null);
	});
}