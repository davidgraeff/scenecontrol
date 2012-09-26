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
	}
};

// Global methods for scenes
Scenes = {
	delete: function(sceneid) {
		console.log("delete scene", sceneid);
	},
	create: function() {
		var newscene = {"type_": "scene","categories": [],"enabled": true,"name": name};
		console.log("create scene", newscene);
	}
};
