function collectionsO() { 
	that = this;
	this.collid = null;
	this.serviceid = null;
	this.services = null;
	this.collections = null;
	
	this.currentcollection = function() {
		if (!this.collections || !this.collid)
			return null;
		return this.collections[this.collid];
	}
	
	this.currentservice = function() {
		if (!this.services || !this.serviceid)
			return null;
		return this.services[this.serviceid];
	}
	
	this.addcollection = function(name) {
		$db.saveDoc({"type_": "collection","categories": [],"enabled": true,"name": name}, {  
			success: function(data) {
				$(that).trigger("add_result", true);
			},
			error: function(data) {
				$(that).trigger("add_result", false);
			}
		});
		return this;
	}
	
	this.rmcollection = function() {
		var c = this.currentcollection();
		if (!c)
			return this;
		
		$db.removeDoc({_id:c._id, _rev:c._rev}, {  
			success: function(data) {
				$(that).trigger("rm_result", true);
			},
			error: function(data) {
				$(that).trigger("rm_result", false);
			}
		});
		return this;
	}
	
	this.editcollection = function(c) {
		$db.saveDoc(c, {  
			success: function(data) {
				$(that).trigger("edit_result", true);
			},
			error: function(data) {
				$(that).trigger("edit_result", false);
			}
		});
		return this;
	}

	this.execcollection = function() {
		if (!this.collid)
			return this;
		propertiesWebsocketInstance.write({"plugin_":"CollectionController", "type_":"execute", "member_":"collection.execute", "collectionid":this.collid});
		return this;
	}

	this.refreshCollections = function() {
		$db.view("roomcontrol/collections", {  
			success: function(data) {  
				that.collections = {};
				for (var index in data.rows) {  
					var row = data.rows[index];
					if (row.value === null)
						continue;
					that.collections[row.key] = row.value;
				}
				$(that).trigger("collectionsReady");
		}});
		return this;
	}

	this.refreshServices = function() {
		var c = this.currentcollection();
		if (!c)
			return;
		
		$db.view("roomcontrol/services", {  
			key: c._id,
			success: function(data) {  
				that.services = {};
				console.log(data, c);
				for (var index in data.rows) {  
					var row = data.rows[index];
					if (row.value === null)
						continue;
					that.services[row.key] = row.value;
				}
				$(that).trigger("servicesReady");
			}
		});
	}
	
	this.updateCurrentCollectionAndService = function( e, data ) {
		var $_GET = {};
		
		window.location.search.replace(/\??(?:([^=]+)=([^&]*)&?)/g, function () {
			function decode(s) {
				return decodeURIComponent(s.split("+").join(" "));
			}

			$_GET[decode(arguments[1])] = decode(arguments[2]);
		});
		
		that.collid = $_GET["collectionid"];
		that.serviceid = $_GET["serviceid"];
		console.log("page transition"+ ";" + $_GET["collectionid"], that);
	}
	
	//this.updateCurrentCollectionAndService();
	$(document).bind( "pagechange", this.updateCurrentCollectionAndService);
};
collections = new collectionsO();