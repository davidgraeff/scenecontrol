var mongodb = require('net');
console.log("Start...");

var mongo = require('mongoskin');
var db = mongo.db('localhost:27017/testdb', {w: 1});

db.collection('blog').find().toArray(function (err, items) {
  console.dir(items);
})

db.createCollection("bla", function(err, collection){
    collection.insert({"test":"value"}, function(err2) {});
	console.log("create ok", err);
});