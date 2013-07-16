/**
  * This service interacts with embedded systems which use the firmata firmware (>=2.2)
  * for a standard way of setting digital and analog/pwm output pins and read input pins.
  */

var servicelib = require('../_shared/servicelib.js'), models = servicelib.models;
var firmata = require('firmata');
var board = null;

servicemethods = function() {
	isDigitalPinValue = function(request, args) {
		// first try the outputPins model and use as default/not-found value the inputPins model
		var v = models.outputPins.getValue(args.channel, models.inputPins.getValue(args.channel, false));
		return servicelib.respond(request.id, (v == args.value));
	}

	setDigitalPin = function(request, args) {
		board.digitalWrite(args.channel, args.value);
		models.outputPins.setValue(args.channel, args.value);
		return servicelib.respond(request.id);
	}

	toggleDititalPin = function(request, args) {
		var value = !models.outputPins.getValue(args.channel, false);
		
		board.digitalWrite(args.channel, value);
		models.outputPins.setValue(args.channel, value);
		return servicelib.respond(request.id);
	}

	isPWMValue = function(request, args) {
		var value = models.pwmpins.getValue(args.channel, 0);
		return servicelib.respond(request.id, (value >= args.lower && value <= args.upper));
	}

	setPWM = function(request, args) {
		board.analogWrite(args.channel, args.value);
		models.pwmpins.setValue(args.channel, args.value);
		return servicelib.respond(request.id);
	}

	setPWMRelative = function(request, args) {
		var value = models.pwmpins.getValue(args.channel, 0) + args.value;
		if (value<0) value = 0;
		else if (value>255) value = 255;
		
		board.analogWrite(args.channel, value);
		models.pwmpins.setValue(args.channel, value);
		return servicelib.respond(request.id);
	}
}

servicelib.ready = function() {
	// register to properties
	
	// setup models
	servicelib.addModel("outputPins","channel", {"getValue":"value"},{"setValue":"value"});
	servicelib.addModel("inputPins","channel", {"getValue":"value"},{"setValue":"value"});
	servicelib.addModel("pwmpins","channel", {"getValue":"value"},{"setValue":"value"});
}

servicelib.gotconfiguration = function(config) {
	// full reset
	servicelib.methods = null;
	servicelib.resetModels();

	// init firmata device
	var board = new firmata.Board(config.serialport,function(){
		//arduino is ready to communicate
		console.log("Serial Firmata ready!");
		// setup methods
		servicelib.methods = servicemethods;
		
		for (var i = 0;i<firmata.Board.pins;++i) {
			// current element with the form
			/* {
			 mode://current mode of pin which is on the the board.MODES.
			,value://current value of the pin. when pin is digital and set to output it will be Board.HIGH or Board.LOW.  If the pin is an analog pin it will be an numeric value between 0 and 1023.
			,supportedModes://an array of modes from board.MODES that are supported on this pin.
			,analogChannel://will be 127 for digital pins and the pin number for analog pins.
			}
			*/
			var c = board.pins[i];
			// Sync firmata pin data with models
			if (c.mode == firmata.Board.MODES.OUTPUT) {
				models.outputPins.setValue(i, c.value);
			} else if (c.mode == firmata.Board.MODES.INPUT) {
				models.inputPins.setValue(i, c.value);
				// Model update: input pins get updated by using digitalRead
				board.digitalRead(i,function(newvalue) { models.inputPins.setValue(i, newvalue); } )
			} else if (c.mode == firmata.Board.MODES.PWM) {
				models.pwmpins.setValue(i, c.value);
			}
		}
		console.log("board.pins", board.pins);
	});  
}
