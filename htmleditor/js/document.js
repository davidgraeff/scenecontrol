/**
 * Document Module - Store information about the current active scene and scene item.
 * Available as window.CurrentScene, CurrentSceneItem.
 * Uses modules: Websocket, Storage
 */
(function (window) {
	"use strict";

	// current scene object
	window.CurrentScene = {
		id: null,
		
		set: function(sceneid) {
			CurrentScene.id = sceneid;
		},
		
		isValid: function() {
			return (CurrentScene.id !== null);
		},

		checkscene: function(sceneid, removed) {
			if (!removed && sceneid!=null) {
				return;
			}
			if (CurrentScene.id != sceneid)
				return;
			CurrentScene.id = null;
			$(this).trigger("clear", sceneid);
		},
		
		rename: function(name) {
			var scene = storageInstance.scenes[CurrentScene.id];
			scene.name = name;
			websocketInstance.updateDocument(scene);
		},
		
		getName: function() {
			return storageInstance.scenes[CurrentScene.id].name;
		},
		
		getTags: function() {
			var tags = storageInstance.scenes[CurrentScene.id].categories;
			return (tags instanceof Array) ? tags.join(",") : "";
		},
		
		setTags: function(taglist) {
			var scene = storageInstance.scenes[CurrentScene.id];
			scene.categories = taglist;
			websocketInstance.updateDocument(scene);
		}
	};

	// current scene item object
	window.CurrentSceneItem = {
		item: null,
		schema: null,
		name: "",
		subname: "",
		temporary: false,
		
		set: function(sceneitemdocument, temporary) {
			CurrentSceneItem.item = sceneitemdocument;
			CurrentSceneItem.temporary = temporary;
			var schema = storageInstance.schemaForDocument(sceneitemdocument);
			CurrentSceneItem.name = sceneitemdocument.componentid_;
			if (temporary) {
				CurrentSceneItem.name +=" (Neu, nicht gespeichert)"
			}
			
			if (schema == null) {
				schema = {
					parameters:{
						raw:{name:"Schemalose Daten",type:"rawdoc"}
					}
				};
				CurrentSceneItem.subname = sceneitemdocument.method_;
			} else {
				CurrentSceneItem.subname = schema.name;
			}
			CurrentSceneItem.schema = schema;
		},
		
		createParameterForm: function($ulBase) {
			var doc = CurrentSceneItem.item;
			var schema = CurrentSceneItem.schema;
			var result = true;
			for (var paramid in schema.parameters) {
				var parameter = schema.parameters[paramid];
				var domid = doc.componentid_ + "_" + paramid + "_" + doc.id_;
				var comon = 'id="'+domid+'" name="'+paramid+'" internal-data-type="'+parameter.type+'"';
				var $elem;
				
				if (parameter.type == "rawdoc") {
					var docCopy = jQuery.extend(true, {}, doc);
					delete docCopy.id_;
					delete docCopy.type_;
					delete docCopy.componentid_;
					delete docCopy.instanceid_;
					delete docCopy.method_;
					delete docCopy.sceneid_;
					$elem = $('<li >' + 
					'<label for="'+domid+'">'+parameter.name+'</label>' + 
					'<textarea rows="8" '+comon+'">'+JSON.stringify(docCopy, null, "\t")+'</textarea></li>').appendTo($ulBase);
				} else if (parameter.type == "string") {
					var value = doc[paramid] ? doc[paramid] : parameter.value;
					$elem = $('<li data-role="fieldcontain">' + 
					'<label for="'+domid+'">'+parameter.name+'</label>' + 
					'<input type="text" '+comon+' value="'+value+'"  /></li>').appendTo($ulBase);
				} else if (parameter.type == "date") {
					var value = doc[paramid] ? doc[paramid] : parameter.value;
					$elem = $('<li data-role="fieldcontain">' + 
					'<label for="'+domid+'">'+parameter.name+'</label>' + 
					'<input type="date" '+comon+' value="'+value+'"  /></li>').appendTo($ulBase);
				} else if (parameter.type == "time") {
					var value = doc[paramid] ? doc[paramid] : parameter.value;
					$elem = $('<li data-role="fieldcontain">' + 
					'<label for="'+domid+'">'+parameter.name+'</label>' + 
					'<input type="time" '+comon+' value="'+value+'"  /></li>').appendTo($ulBase);
				} else if (parameter.type == "boolean") {
					var value = doc[paramid] ? doc[paramid] : parameter.value;
					$elem = $('<li data-role="fieldcontain">'+
					'<label for="'+domid+'">'+parameter.name+'</label>'+
					'<select '+comon+' data-role="slider">'+
					'<option value="0" '+(value?'':'selected')+'>Off</option><option value="1" '+(value?'selected':'')+'>On</option>' +
					'</select></li>').appendTo($ulBase);
				} else if (parameter.type == "integer" && parameter.min && parameter.max) {
					var value = doc[paramid] ? doc[paramid] : parameter.value;
					$elem = $('<li data-role="fieldcontain">' +
					'<label for="'+domid+'">'+parameter.name+'</label>' +
					'<input type="range" data-highlight="true" '+comon+' value="'+value+'"' +
					'min="'+parameter.min+'" max="'+parameter.max+'" '+(parameter.step?'step="'+parameter.step+'"':'')+' /></li>').appendTo($ulBase);
				} else if (parameter.type == "integer") {
					var value = doc[paramid] ? doc[paramid] : parameter.value;
					$elem = $('<li data-role="fieldcontain">' +
					'<label for="'+domid+'">'+parameter.name+'</label>' +
					'<input type="number" '+comon+' value="'+value+'" '+
					(parameter.step?'step="'+parameter.step+'" ':'')+
					(parameter.min?'min="'+parameter.min+'" ':'')+
					(parameter.max?'max="'+parameter.max+'" ':'')+
					' /></li>').appendTo($ulBase);
				} else if (parameter.type == "enum") {
					var d = '<li data-role="fieldcontain">' +
					'<fieldset data-role="controlgroup" id="'+domid+'">' +
					'	<legend>'+parameter.name+'</legend>';
					
					for (var i=0;i<parameter.data.length;++i) {
						var itemDomID = domid + i;
						d+= '<input type="radio" name="'+paramid+'" id="'+itemDomID+'" internal-data-type="'+parameter.type+'" value="'+i+'" '+(i==doc[paramid]?'checked="checked"':'')+'>' +
						'<label for="'+itemDomID+'">'+parameter.data[i]+'</label>';
					}
					
					d += '</fieldset>' +
					'</li>';
					
					$elem = $(d).appendTo($ulBase);
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
							var s = key==doc[paramid];
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
					
					$elem = $(d).appendTo($ulBase);
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
					
					$elem = $(d).appendTo($ulBase);
				} else {
					$elem = $('<li>Unknown parameter: '+parameter.type+'</li>').appendTo($ulBase);
					console.log("unknown parameter", parameter);
					result = false;
				}

				// Set outer element to the actuall element
				$elem = $elem.find('[name='+paramid+']');
				
				// Call a method of the plugin, if this parameter changes
				if (parameter.onchange) {

					$elem.data("paramdata", parameter);
					$elem.data("paramid", paramid);
					$elem.on('input', function() {
						CurrentSceneItem.propertyOnChange($(this).data("paramdata").onchange, $(this).attr('internal-data-type'), $(this).val(), $(this).data("paramid"));
					});
					$elem.on('change', function() {
						CurrentSceneItem.propertyOnChange($(this).data("paramdata").onchange, $(this).attr('internal-data-type'), $(this).val(), $(this).data("paramid"));
					});
				}
				
				// Listen for a property change message and set the properties text to this element
				if (parameter.notification && parameter.indexproperty && parameter.textproperty) {
					$elem.data("paramdata", parameter);
					$elem.data("paramid", paramid);
					$elem.data("notification", parameter.notification);
				}
				
			} // for loop
			return result;
		},
		
		propertyOnChange: function(destMethod, type, value, name) {
			var dataout = {"method_":destMethod,"type_":"execute","componentid_":CurrentSceneItem.item.componentid_, "instanceid_":CurrentSceneItem.item.instanceid_};
			CurrentSceneItem.serializeElement(dataout, type, value, name);
	// 		console.log("propertyOnChange "+paramid, destMethod, o);
			websocketInstance.write(dataout);
		},
		
		registerChangeNotifiers: function($ulBase, callbackInputChanged) {
			$ulBase.find('input,textarea').not(':checkbox,:radio').on('input', function() {
				if (callbackInputChanged)
					callbackInputChanged();
			});
			$ulBase.find('select,input[type=checkbox],input[type=radio],input[type=range],input[type=number]').on('change', function() {
				if (callbackInputChanged)
					callbackInputChanged();
			});
		},
		
		notification: function($form, doc) {
			var a = $form.serializeArray();
			$.each(a, function() {
				var $elem = $form.find('[name='+this.name+']');
				if ($elem.data("notification")==doc.id_) {
					var textprop = $elem.data("paramdata").textproperty;
					$elem.val(doc[textprop]);
				}
			});
		}, 
		
		serializeForm: function($form) {
			var dataout = {};
			var invalidfields = [];
			var a = $form.serializeArray();
			
			$.each(a, function() {
				var $elem = $form.find('[name='+this.name+'][internal-data-type]');
				var type = $elem.attr('internal-data-type');
				var value = $elem.val();
				var size = $elem.attr('data-size'); // only for multi enums

				if (!CurrentSceneItem.serializeElement(dataout, type, value, this.name, size)) {
					invalidfields.push(this.name);
				}
			});
			return {"data":dataout,"invalid":invalidfields};
		}, 
		
		serializeElement: function(o, type, value, name, datasize) {
			if (type=="rawdoc") {
				var p = JSON.parse(value);
				// security: do not allow server-used key names for raw fields
				delete p.id_;
				delete p.type_;
				delete p.componentid_;
				delete p.instanceid_;
				delete p.method_;
				delete p.sceneid_;
				o = jQuery.extend(true, o, p);
			} else if (type=="boolean") {
				o[name] = (value=="1")?true:false;
			} else if (type=="integer") {
				o[name] = parseInt( value );
			} else if (type=="enum") {
				if (value=="") return false;
				o[name] = parseInt( value );
			} else if (type == "modelenum") {
				if (value=="") return false;
				o[name] = value;
				//console.log("save modelenum item", value, name);
			} else if (type=="multienum") {
				if (!o[name]) {
					o[name] = [];
					for (var i=0;i<datasize;++i) {
						o[name][i] = false;
					}
				}
				//console.log("save multienum item", parseInt(this.value), o[this.name].length);
				o[name][parseInt(value)] = true;
			} else {
				o[name] = value;
			}
			return true;
		}
	};

	window.SceneItem = {
		text: function(sceneItem, schema) {
			// create a copy of the scene item
			var sceneItemCopy = {};
			$.extend(true, sceneItemCopy, sceneItem);
			// extend the object: replace enum integers by their text representations, fill in model data etc
			for (var paramid in schema.parameters) {
				var parameter = schema.parameters[paramid];
				if (parameter.type == "boolean") {
					sceneItemCopy[paramid] = sceneItemCopy[paramid] ? "An" : "Aus";
				} else if (parameter.type == "enum") {
					for (var i=0;i<parameter.data.length;++i) {
						if (i==sceneItemCopy[paramid]) {
							sceneItemCopy[paramid] = parameter.data[i];
							break;
						}
					}
				} else if (parameter.type == "multienum") {
					var counter = 0;
					var dataFromDoc = sceneItemCopy[paramid];
					var resultString = "";
					for (var index in parameter.data) {
						if (dataFromDoc && dataFromDoc.length>counter && dataFromDoc[counter] === true) {
							resultString += parameter.data[index] + " ";
						}
						++counter;
					}
					sceneItemCopy[paramid] = resultString;
				} else if (parameter.type == "modelenum") {
					var model = storageInstance.modelItems(sceneItemCopy.componentid_,sceneItemCopy.instanceid_,parameter.model);
					if (!model || !model.data) {
						continue;
					}
					var elems = model.data;
					for (var index in elems) {
						if (elems.hasOwnProperty(index)) {
							var modelitem = elems[index];
							var key = modelitem[parameter.indexproperty];
							var value = modelitem[parameter.textproperty];
							if (key==sceneItemCopy[paramid]) {
								sceneItemCopy[paramid] = value; // show value instead of key
								break;
							}
						}
					}
				}
			}
			// create a template object that we pass in the context (properties from the scene item).
			// All {{SOMETHING}} tags are replaced by corresponding values.
			var template = Handlebars.compile(schema.text);
			return template(sceneItemCopy);
		}
	};
})(window);
