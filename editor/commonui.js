
function createParameterForm($ulBase, schema, doc, callbackInputChanged) {
	for (var paramid in schema.parameters) {
		var parameter = schema.parameters[paramid];
		var domid = doc.componentid_ + "_" + paramid + "_" + doc.id_;
		if (parameter.type == "rawdoc") {
			$ulBase.append('<li >' + 
			'<label for="'+domid+'">'+parameter.name+'</label>' + 
			'<textarea rows="8" id="'+domid+'" name="'+paramid+'">'+JSON.stringify(doc, null, "\t")+'</textarea></li>');
		} else if (parameter.type == "string") {
			var value = doc[paramid] ? doc[paramid] : parameter.value;
			$ulBase.append('<li data-role="fieldcontain">' + 
			'<label for="'+domid+'">'+parameter.name+'</label>' + 
			'<input type="text" id="'+domid+'" name="'+paramid+'" value="'+value+'"  /></li>');
		} else if (parameter.type == "boolean") {
			var value = doc[paramid] ? doc[paramid] : parameter.value;
			$ulBase.append('<li data-role="fieldcontain">'+
			'<label for="'+domid+'">'+parameter.name+'</label>'+
			'<select id="'+domid+'" name="'+paramid+'" data-role="slider">'+
			'<option value="0" '+(value?'':'selected')+'>Off</option><option value="1" '+(value?'selected':'')+'>On</option>' +
			'</select></li>');
		} else if (parameter.type == "integer" && parameter.min && parameter.max) {
			var value = doc[paramid] ? doc[paramid] : parameter.value;
			$ulBase.append('<li data-role="fieldcontain">' +
			'<label for="'+domid+'">'+parameter.name+'</label>' +
			'<input type="range" data-highlight="true" id="'+domid+'" name="'+paramid+'" value="'+value+'"' +
			'min="'+parameter.min+'" max="'+parameter.max+'" '+(parameter.step?'step="'+parameter.step+'"':'')+' /></li>');
		} else if (parameter.type == "integer") {
			var value = doc[paramid] ? doc[paramid] : parameter.value;
			$ulBase.append('<li data-role="fieldcontain">' +
			'<label for="'+domid+'">'+parameter.name+'</label>' +
			'<input type="number" id="'+domid+'" name="'+paramid+'" value="'+value+'" '+
			(parameter.step?'step="'+parameter.step+'" ':'')+
			(parameter.min?'min="'+parameter.min+'" ':'')+
			(parameter.max?'max="'+parameter.max+'" ':'')+
			' /></li>');
		} else if (parameter.type == "enum") {
			var d = '<li data-role="fieldcontain">' +
			'<fieldset data-role="controlgroup" id="'+domid+'">' +
			'	<legend>'+parameter.name+'</legend>';
			
			for (var i=0;i<parameter.data.length;++i) {
				var itemDomID = domid + i;
				d+= '<input type="radio" name="'+paramid+'" id="'+itemDomID+'" value="'+parameter.data[i]+'" '+(i==doc[paramid]?'checked="checked"':'')+' />' +
				'<label for="'+itemDomID+'">'+parameter.data[i]+'</label>';
			}
			
			d += '</fieldset>' +
			'</li>';
			
			$ulBase.append(d);
		} else if (parameter.type == "modelenum") {
			var d = '<li data-role="fieldcontain">' +
			'<label for="'+domid+'" class="select">'+parameter.name+'</label>' +
			'<select name="'+paramid+'" id="'+domid+'" data-native-menu="false">';
			
			var dElements;
			
			var model = storageInstance.modelItems(doc.componentid_,doc.instanceid_,parameter.model);
			var elems = model.data;
			//console.log("parameter_modelenum: "+paramid, parameter, elems);
			var counter = 0;
			var selected = 0;
			for (var index in elems) {
				if (elems.hasOwnProperty(index)) {
					var key = elems[index][parameter.indexproperty];
					var value = elems[index][parameter.textproperty];
					var s = key==doc[parameter.indexproperty];
					if (s)
						++selected;
					dElements += '<option value="'+key+'" '+(s?'selected':'')+'>'+value+'</option>';
					++counter;
				}
			}
			
			if (!counter) {
				d += '<option>'+parameter.name+': Keine Daten</option>';
			} else if (!selected) {
				d += '<option>'+parameter.name+': Keine Auswahl!</option>';
				d += dElements;
			} else {
				d += dElements;
			}
			
			d += '</select>' +
			'</li>';
			
			$ulBase.append(d);
		} else {
			console.log("unknown parameter", parameter);
		}
	}
}

function registerChangeNotifiers($ulBase, callbackInputChanged) {
	$ulBase.find('input,textarea').not(':checkbox,:radio').bind('input', function() {
		if (callbackInputChanged)
			callbackInputChanged();
		$ulBase.find(".btnsaveitem").removeClass("ui-disabled");
	});
	$ulBase.find('select,input[type=checkbox],input[type=radio],input[type=range],input[type=number]').bind('change', function() {
		if (callbackInputChanged)
			callbackInputChanged();
		$ulBase.find(".btnsaveitem").removeClass("ui-disabled");
	});
}

$.fn.serializeObject = function()
{
	var o = {};
	var a = this.serializeArray();
	$.each(a, function() {
		if (o[this.name] !== undefined) {
			if (!o[this.name].push) {
				o[this.name] = [o[this.name]];
			}
			o[this.name].push(this.value || '');
		} else {
			o[this.name] = this.value || '';
		}
	});
	return o;
};
