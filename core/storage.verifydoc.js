
function baseName(str)
{
	var base = new String(str).substring(str.lastIndexOf('/') + 1); 
	if(base.lastIndexOf(".") != -1)       
		base = base.substring(0, base.lastIndexOf("."));
	return base;
}
function dirName(str)
{
	var i = str.lastIndexOf('/')-1; i = str.lastIndexOf('/', i);
	var base = new String(str).substring(i + 1); 
	if(base.lastIndexOf("/") != -1)       
		base = base.substring(0, base.lastIndexOf("/"));
	return base;
}


/****** Verifier *********/
exports.genericverifier = function(doc) {
	this.setIDIfNotExist = function(doc, filename) {
		if (!doc.componentid_)
			doc.componentid_ = dirName(filename);
		
		if (!doc.id_) {
			if (doc.type_=="configuration")
				doc.id_ = doc.componentid_ + "." + doc.instanceid_;
			else if (doc.type_=="description")
				doc.id_ = doc.componentid_ + "." + doc.language;
			else if (doc.type_=="schema")
				doc.id_ = doc.componentid_ + "." + doc.method_;
			else
				doc.id_ = baseName(filename);
		}
	}
	this.isValid = function(doc) {
		var valid_types = ["event","condition","action","configuration","schema","description"];
		if (doc.type_=="description" && (!doc.language || !doc.name)) {
			return false;
		}
		if (doc.type_=="scene" && (!doc.name)) {
			return false;
		}
		if (doc.type_=="configuration" && !doc.instanceid_) {
			return false;
		}
		return (doc.componentid_ && doc.id_);
	}
};