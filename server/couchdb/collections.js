function collectionsO() { 
	that = this;
	this.collid = localStorage.getItem('collid');
	this.collections = null;
	this.callbackOnCollection = {};
	
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
		//propertiesWebsocketInstance.write({"plugin_":"CollectionController", "type_":"execute", "member_":"collection.execute", "collectionid":this.collid});
		return this;
	}

	this.getCollections = function(callback) {
		if (that.collections == 1)
			return;
		
		// set to 1 to ignore all further requests
		that.collections = 1;
		
		$db.view("_server/collections", {  
			success: function(data) {  
				that.collections = {};
				for (var index in data.rows) {  
					var row = data.rows[index];
					if (row.value === null)
						continue;
					that.collections[row.id] = row.value;
					console.log("collection found", row);
					if (that.callbackOnCollection[row.key]) {
						// call each callback that is registered to this id
						for ( var i=0, len=that.callbackOnCollection[row.key].length; i<len; ++i ){
							// call callback
							that.callbackOnCollection[row.key][i](row.value);
						}
						// remove all callbacks of this id
						delete that.callbackOnCollection[row.key];
					}
				}
				if (typeof callback == "function")
					callback(that.collections);
			},
			error: function(event) {
				that.collections = null;
			}
		});
		return this;
	}

	/**
	 * Callback function will be called with all services of the collection with id == collid
	 */
	this.getServices = function(callback, collid) {
		if (typeof callback != "function")
			return;
		
		$db.view("_server/services", {  
			key: collid,
			success: function(data) {  
				var services = {};
				for (var index in data.rows) {  
					var row = data.rows[index];
					console.log("service found", row);
					if (row.value === null)
						continue;
					services[row.id] = row.value;
				}
				callback(services, collid);
			}
		});
	}
	
	/**
	 * Callback function will be called as soon as the collection with id == collid has been loaded
	 */
	this.registerCollectionListener = function(callback, collid) {
		if (typeof callback != "function")
			return;
		
		// if collection already available immediatelly call callback
		if (that.collections != null && that.collections[collid]) {
			callback(that.collections[collid]);
		} else if (that.callbackOnCollection[collid]) { // there are already entries
			that.callbackOnCollection[collid].push(callback);
		} else
			that.callbackOnCollection[collid] = [callback]
		
		// if no collections were requested so far, do this now
		if (that.collections == null) {
			that.getCollections();
		}
	}
	
	this.setCollection = function(collid) {
	    localStorage.setItem('collid', collid);
	    that.collid = collid;
	    console.log("setCollid", collid, localStorage.getItem('collid'));
	}
};
collections = new collectionsO();