// current scene object
CurrentScene = {
	id: null,
	
	set: function(sceneid) {
		CurrentScene.id = sceneid;
	},
	
	newevent: function() {
		
	},

	newcondition: function() {
		
	},

	newaction: function() {
		
	},

	isValid: function() {
		return (CurrentScene.id !== null);
	},

	checkscene: function(sceneid, removed) {
		if (!removed && sceneid!=null) {
			return;
		}
		if (CurrentScene.id != sceneid)
			return;
		CurrentScene.id = null;
		$(this).trigger("clear", sceneid);
	},
	
	rename: function(name) {
		var scene = storageInstance.scenes[storageInstance.unqiueSceneID(CurrentScene.id)];
		scene.name = name;
		Document.change(scene);
	},
	
	getName: function() {
		return storageInstance.scenes[storageInstance.unqiueSceneID(CurrentScene.id)].name;
	},
	
	getTags: function() {
		var cats = storageInstance.scenes[storageInstance.unqiueSceneID(CurrentScene.id)].categories;
		return (cats instanceof Array) ? cats.join(",") : "";
	},
	
	setTags: function(taglist) {
		var scene = storageInstance.scenes[storageInstance.unqiueSceneID(CurrentScene.id)];
		scene.categories = taglist;
		Document.change(scene);
	}
};

// Global methods for scenes
Document = {
	createScene: function(name) {
		var newscene = {"id_":"GENERATEGUID", "componentid_":"server", "type_": "scene","categories": [],"enabled": true,"name": name};
		Document.change(newscene);
	},
	
	createConfig: function(instanceid, componentid) {
		var newconfig = {"id_":"GENERATEGUID", "componentid_":componentid, "type_": "configuration","instanceid_": instanceid};
		Document.change(newconfig);
	},
	
	remove: function(sceneDocument) {
		console.log("Remove: ", sceneDocument)
		websocketInstance.write({"componentid_":"server","type_":"execute","method_":"removeDocument","doc":sceneDocument});
	},
	
	change: function(sceneDocument) {
		console.log("Change: ", sceneDocument)
		websocketInstance.write({"componentid_":"server","type_":"execute","method_":"changeDocument","doc":sceneDocument});
	}
};
