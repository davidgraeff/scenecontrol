// Logging
exports.init = function(modulename) {
	function fnt(orig) {
		this.orig = orig;
		this.fnt = function() {
			var date = new Date();
			var prefix = "["+date.getHours()+":"+date.getMinutes()+", "+modulename+"]";
			var msgs = [prefix];
			while(arguments.length)
				msgs.push([].shift.call(arguments));
			orig.apply(console, msgs);
		}
	};
	
	exports.origLog = console.log;
	exports.origWarn = console.warn;
	exports.origErr = console.error;
	
	origLog = new fnt(console.log);
	console.log = origLog.fnt;
	origWarn = new fnt(console.warn);
	console.warn = origWarn.fnt;
	origErr = new fnt(console.error);
	console.error = origErr.fnt;
}
