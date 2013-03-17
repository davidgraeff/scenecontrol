
exports.variables = {};

exports.setVariable = function(id, value) {
	exports.variables[id] = value;
}

exports.getVariable = function(id) {
	return exports.variables[id];
}

exports.apply = function(propName) {
	for (var id in exports.variables) {
		if (id == propName)
			return exports.variables[id];
	}
	return null;
}