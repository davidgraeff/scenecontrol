var mongo = require('mongoskin');
var configs = require('./config.js');
var db = mongo.db('localhost:27017/'+configs.runtimeconfig.databasename, {w: 1});
// 
// db.collection('blog').find().toArray(function (err, items) {
// 	console.dir(items);
// })
// 
// db.createCollection("bla", function(err, collection){
// 	collection.insert({"test":"value"}, function(err2) {});
// 	console.log("create ok", err);
// 	});

var importpaths = [];

exports.addImportPath = function(path) {importpaths.push(path);}

exports.init = function(callback) {
	callback(null, null);
}

exports.importNewFiles = function(callback) {
	callback(null, null);
}


exports.load = function(callback) {
	callback(null, null);
}
