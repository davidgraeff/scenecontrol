
function createParameterForm($ulBase, schema, doc, callbackInputChanged) {
	var result = true;
	for (var paramid in schema.parameters) {
		var parameter = schema.parameters[paramid];
		var domid = doc.componentid_ + "_" + paramid + "_" + doc.id_;
		var comon = 'id="'+domid+'" name="'+paramid+'" data-type="'+parameter.type+'"';
		
		if (parameter.type == "rawdoc") {
			var docCopy = jQuery.extend(true, {}, doc);
			delete docCopy.id_;
			delete docCopy.type_;
			delete docCopy.componentid_;
			delete docCopy.instanceid_;
			delete docCopy.method_;
			delete docCopy.sceneid_;
			$ulBase.append('<li >' + 
			'<label for="'+domid+'">'+parameter.name+'</label>' + 
			'<textarea rows="8" '+comon+'">'+JSON.stringify(docCopy, null, "\t")+'</textarea></li>');
		} else if (parameter.type == "string") {
			var value = doc[paramid] ? doc[paramid] : parameter.value;
			$ulBase.append('<li data-role="fieldcontain">' + 
			'<label for="'+domid+'">'+parameter.name+'</label>' + 
			'<input type="text" '+comon+' value="'+value+'"  /></li>');
		} else if (parameter.type == "date") {
			var value = doc[paramid] ? doc[paramid] : parameter.value;
			$ulBase.append('<li data-role="fieldcontain">' + 
			'<label for="'+domid+'">'+parameter.name+'</label>' + 
			'<input type="date" '+comon+' value="'+value+'"  /></li>');
		} else if (parameter.type == "time") {
			var value = doc[paramid] ? doc[paramid] : parameter.value;
			$ulBase.append('<li data-role="fieldcontain">' + 
			'<label for="'+domid+'">'+parameter.name+'</label>' + 
			'<input type="time" '+comon+' value="'+value+'"  /></li>');
		} else if (parameter.type == "boolean") {
			var value = doc[paramid] ? doc[paramid] : parameter.value;
			$ulBase.append('<li data-role="fieldcontain">'+
			'<label for="'+domid+'">'+parameter.name+'</label>'+
			'<select '+comon+' data-role="slider">'+
			'<option value="0" '+(value?'':'selected')+'>Off</option><option value="1" '+(value?'selected':'')+'>On</option>' +
			'</select></li>');
		} else if (parameter.type == "integer" && parameter.min && parameter.max) {
			var value = doc[paramid] ? doc[paramid] : parameter.value;
			$ulBase.append('<li data-role="fieldcontain">' +
			'<label for="'+domid+'">'+parameter.name+'</label>' +
			'<input type="range" data-highlight="true" '+comon+' value="'+value+'"' +
			'min="'+parameter.min+'" max="'+parameter.max+'" '+(parameter.step?'step="'+parameter.step+'"':'')+' /></li>');
		} else if (parameter.type == "integer") {
			var value = doc[paramid] ? doc[paramid] : parameter.value;
			$ulBase.append('<li data-role="fieldcontain">' +
			'<label for="'+domid+'">'+parameter.name+'</label>' +
			'<input type="number" '+comon+' value="'+value+'" '+
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
				d+= '<input type="radio" name="'+paramid+'" id="'+itemDomID+'" data-type="'+parameter.type+'" value="'+i+'" '+(i==doc[paramid]?'checked="checked"':'')+'>' +
				'<label for="'+itemDomID+'">'+parameter.data[i]+'</label>';
			}
			
			d += '</fieldset>' +
			'</li>';
			
			$ulBase.append(d);
		} else if (parameter.type == "modelenum") {
			var d = '<li data-role="fieldcontain">' +
			'<label for="'+domid+'" class="select">'+parameter.name+'</label>' +
			'<select '+comon+' data-native-menu="false">';
			
			var model = storageInstance.modelItems(doc.componentid_,doc.instanceid_,parameter.model);
			if (!model || !model.data) {
				console.warn("Model does not exist:", parameter.model)
				result = false;
				continue;
			}
			var elems = model.data;
			var counter = 0;
			var selected = 0;
			var dElements;
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
				d += '<option value="">'+parameter.name+': Keine Daten</option>';
			} else if (!selected) {
				d += '<option value="">'+parameter.name+': Keine Auswahl!</option>';
				d += dElements;
			} else {
				d += dElements;
			}
			
			d += '</select>' +
			'</li>';
			
			$ulBase.append(d);
		} else if (parameter.type == "multienum") {
			var d = '<li data-role="fieldcontain">' +
			'<label for="'+domid+'" class="select">'+parameter.name+'</label>' +
			'<select '+comon+' data-size="'+parameter.data.length+'" data-native-menu="false" multiple="multiple">';
			var counter = 0;
			var dataFromDoc = doc[paramid];
			for (var index in parameter.data) {
				var s = false;
				if (dataFromDoc && dataFromDoc.length>counter && dataFromDoc[counter] === true)
					s = true;
				d += '<option value="'+counter+'" '+(s?'selected':'')+'>'+parameter.data[index]+'</option>';
				++counter;
			}
			d += '</select>' +
			'</li>';
			
			$ulBase.append(d);
		} else {
			$ulBase.append('<li>Unknown parameter: '+parameter.type+'</li>');
			console.log("unknown parameter", parameter);
			result = false;
		}
	}
	return result;
}

function registerChangeNotifiers($ulBase, callbackInputChanged) {
	$ulBase.find('input,textarea').not(':checkbox,:radio').on('input', function() {
		if (callbackInputChanged)
			callbackInputChanged($ulBase);
	});
	$ulBase.find('select,input[type=checkbox],input[type=radio],input[type=range],input[type=number]').on('change', function() {
		if (callbackInputChanged)
			callbackInputChanged($ulBase);
	});
}

$(websocketInstance).on('onclose', function() {
	window.location = window.location.href.replace( /#.*/, "");
});

function serializeForm($form)
{
	var o = {};
	var invalidfields = [];
	var a = $form.serializeArray();
	$.each(a, function() {
		if (this.value=="") {
			invalidfields.push(this.name);
			return;
		}
		var type = $form.find('[name='+this.name+']').attr('data-type');
		if (type=="rawdoc") {
			var p = JSON.parse(this.value);
			// security: do not allow server-used key names for raw fields
			delete p.id_;
			delete p.type_;
			delete p.componentid_;
			delete p.instanceid_;
			delete p.method_;
			delete p.sceneid_;
			o = jQuery.extend(true, o, p);
		} else if (type=="boolean") {
			o[this.name] = (this.value=="1")?true:false;
		} else if (type=="integer") {
			o[this.name] = parseInt( this.value );
		} else if (type=="enum") {
			o[this.name] = parseInt( this.value );
		} else if (type=="multienum") {
			var size = $form.find('[name='+this.name+']').attr('data-size');
			if (!o[this.name]) {
				o[this.name] = [];
				for (var i=0;i<size;++i) {
					o[this.name][i] = false;
				}
			}
			//console.log("save multienum item", parseInt(this.value), o[this.name].length);
			o[this.name][parseInt(this.value)] = true;
		// } else if (o[this.name] !== undefined) {
			// if (!o[this.name].push) {
				// o[this.name] = [o[this.name]];
			// }
			// o[this.name].push(this.value);
		} else {
			o[this.name] = this.value;
		}
	});
	return {"data":o,"invalid":invalidfields};
};
