var vows = require('vows'), assert = require('assert');
var controlflow = require('async');

// Prepare
/**
 * Add scene + scene item [core service (setVariable)]
 */

// Test executing scene (+ Test emitted properties, + Test acks)

// Test resulting variable

// Test executing scene item for getting variable content

// Test calling non existant service


/// Tests
setTimeout(function() {
	var clientcomEmu = function() {
		this.info = { sessionid: "testservice",componentid_:"testservice" };
		this.send = function(data) {
			console.log("  Execute result:", data);
		}
	}
	var c = new clientcomEmu();
	var services = require("./services.js");
	console.log("Test...");
	services.servicecall({requestid_:"teststart",doc:{componentid_:"core", instanceid_:"main",method_:"startscene", sceneid_:"54114e2456df4aa2a15b161a47a3d8d1"}}, c);
	services.servicecall({requestid_:"testled",doc:{componentid_:"scenecontrol.leds", instanceid_:"null",method_:"setLed", channel:"1", value:255, fade:0}}, c);
}, 2000);