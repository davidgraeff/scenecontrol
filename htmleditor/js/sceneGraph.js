/**
 * Scene Canvas Module - Contains all nodes, links objects. Create nodes/links from scene document.
 * Available as window.sceneCanvas
 * Uses modules: Canvas
 */
(function (window) {
	"use strict";
	
	window.sceneCanvas = function() {
		this.that = this;
		this.scene = null;
		this.nodes = [];
	}
	
	window.sceneNode = function(data, nodedata, nodeindex) {
		this.that = this;
		this.nodeindex = nodeindex;
		// Directions
		this.linkedIn = {top:null, bottom:null, left:null, right:null};
		this.linkedOut = {top:null, bottom:null, left:null, right:null};
		// position
		if (nodedata.editorprop) {
			this.x = nodedata.editorprop.x;
			this.y = nodedata.editorprop.y;
		}
		// outgoing edges
		for (var i in nodedata.e) {
			if (nodedata.e[i]) {
				this.linkedOut[i] = nodedata.e[i];
			}
		}
		
		this.removeAllLinks = function() {
			this.linkedIn = {top:null, bottom:null, left:null, right:null};
			this.linkedOut = {top:null, bottom:null, left:null, right:null};
		}
		
		this.removeLinked = function(sceneItemDocument) {
			var result = false;
			var k = Object.keys(this.linkedIn);
			for (var i in k) { i = k[i];
				if (!this.linkedIn[i])
					continue;
				if (api.uidEqual(this.linkedIn[i], sceneItemDocument)) {
					this.linkedIn[i] = null;
					result = true;
				}
			}
			
			k = Object.keys(this.linkedOut);
			for (var i in k) { i = k[i];
				if (!this.linkedOut[i])
					continue;
				if (api.uidEqual(this.linkedOut[i], sceneItemDocument)) {
					this.linkedOut[i] = null;
					result = true;
				}
			}
			
			return result;
		}
		
		this.setData = function(d) {
			this.data = d;
			this.dataschema = storageInstance.schemaForDocument(d);
			var text;
			if (this.dataschema) {
				if (this.dataschema.text) {
					text = SceneItem.text( this.data, this.dataschema );
				} else {
					text = this.dataschema.name;
				}
			} else {
				// No schema: We do not know how to express this item in text
				text = JSON.stringify(this.data);
			}
			this.text = text;
		}
		this.setData(data);
	}

	window.sceneNode.prototype = {
		enabled: true,
		x: null,
		y: null,
		dropDestination: false,
		setEnable: function(b) {
			this.enabled = b;
		}
	}
	
	window.sceneCanvas.prototype = {
		ctx: null, // handlebar compilation
		disable: function(sceneItemDocument) {
			for (var i = 0; i < this.nodes.length; i++) {
				if (this.nodes[i].data == sceneItemDocument) {
					this.nodes[i].setEnable(false);
				}
			}
		},
		
		indexOfSceneItem: function(sceneItemDocument) {
			for (var i = 0; i < this.nodes.length; i++) {
				var nodedata = this.nodes[i].data;
				if (nodedata.type_==sceneItemDocument.type_ && nodedata.id_==sceneItemDocument.id_)
					return i;
			}
			return -1;
		},
 
		sceneitemchanged: function(sceneItemDocument) {
			var i = this.indexOfSceneItem(sceneItemDocument);
			if (i==-1) {
				return;
			}
			var node = this.nodes[i];
			node.setData(sceneItemDocument);
			node.setEnable(true);
			this.updateNode(node);
		},

		load: function(scene) {
			// variables
			if (!this.ctx)
				this.ctx = Handlebars.compile($("#scenegraphitem-template").html());

			this.unload();
			this.scene = scene;
			var that = this.that;

			// connect event handlers
			$(storageInstance).on('onevent.sceneitems', function(d, flags) {
				if (flags.doc.sceneid_ != that.scene.id_)
					return;
				
				if (!flags.removed)
					that.sceneitemchanged(flags.doc, flags.temporary);
			});

			$(storageInstance).on('oncondition.sceneitems', function(d, flags) {
				if (flags.doc.sceneid_ != that.scene.id_)
					return;
				
				if (!flags.removed)
					that.sceneitemchanged(flags.doc, flags.temporary);
			});

			$(storageInstance).on('onaction.sceneitems', function(d, flags) {
				if (flags.doc.sceneid_ != that.scene.id_)
					return;
				
				if (!flags.removed)
					that.sceneitemchanged(flags.doc, flags.temporary);
			});

			// create node elements
			for (var i = 0; i < scene.v.length; i++) {
				var sceneNodeData = scene.v[i];
				var sceneItem = storageInstance.getDocument(sceneNodeData.type_, sceneNodeData.id_);
				if (sceneItem==null) {
					console.warn("Cannot find scene item "+sceneNodeData.type_+ " " + sceneNodeData.id_);
					continue;
				}
				var node = new sceneNode(sceneItem, sceneNodeData , i);
				this.nodes.push(node);
			}
			
			// transform node link information
			// every node should now its predessor nodes
			this.calcPredecessorNodes();
			
			for (var i = 0; i <this.nodes.length; i++) {
				var node = this.nodes[i];
				// place node in table
				this.updateNode(node, this.placeNode(node));
			}
			
			// init drag lib
			var rd = REDIPS.drag;
			REDIPS.drag.style.borderEnabled = 'none';
			REDIPS.drag.style.borderDisabled = 'none';
			REDIPS.drag.dropMode = 'overwrite';
			
			rd.event.dropped = function () {
				var $destelem = $(rd.obj).parent();
				var targetnode = that.nodes[$(rd.obj).attr("data-nodeindex")];
				that.updateNodeLinkInfo(targetnode, $destelem);
			}
			rd.event.deleted = function () {
				var $destelem = $(rd.obj).parent();
				var targetnode = that.nodes[$(rd.obj).attr("data-nodeindex")];
				var data = targetnode.data;
				$(that).trigger("itemremove", data);
			}
			rd.event.clicked = function (currentCell) {
				var $destelem = $(currentCell), x = $destelem.index(), y = $destelem.parent().index();
				
				// mark drop destinations
				for (var i = 0; i <that.nodes.length; i++) {
					var node = that.nodes[i];
					if ((Math.abs(node.x-x)==1 && node.y==y) || (Math.abs(node.y-y)==1 && node.x==x)) {
						node.dropDestination = true;
						that.drawNodeLinkInfo(node, that.placeNode(node));
					}
				}
				
				return true;
			}
			rd.event.finish = function () {
				// remove all drop destination flags and redraw affected elements
				for (var i = 0; i <that.nodes.length; i++) {
					var node = that.nodes[i];
					if (node.dropDestination) {
						node.dropDestination = false;
						that.drawNodeLinkInfo(node, that.placeNode(node));
					}
				}
			}
			rd.event.droppedBefore = function (targetCell) {
				// allow dropping to empty cell
				if (rd.emptyCell(targetCell, 'test')) 
					return true;
				
				// get position of target cell and corresponding node
				var $destelem = $(targetCell), newx = $destelem.index(), newy = $destelem.parent().index();
				var $elem = $(rd.obj).parent(), x = $elem.index(), y = $elem.parent().index();
				var $destnode = $destelem.children().first();
				if ($destnode.length == 0)
					return;
				
				var $node = $elem.children().first();

				
				// only allow dropping on direct neighbours 
				if (!((Math.abs(newx-x)==1 && newy==y) || (Math.abs(newy-y)==1 && newx==x)))
					return false;
				
				var destnode = that.nodes[$destnode.attr("data-nodeindex")];
				var node = that.nodes[$node.attr("data-nodeindex")];
				
				// remove all links between both
				destnode.removeLinked(node.data);
				node.removeLinked(destnode.data);
				
				if (destnode.x + 1 == node.x && destnode.y == node.y) {
					destnode.linkedIn.right=api.uidFromDocument(node.data);
					node.linkedOut.left=api.uidFromDocument(destnode.data);
				} else if (destnode.x - 1 == node.x && destnode.y == node.y) {
					destnode.linkedIn.left=api.uidFromDocument(node.data);
					node.linkedOut.right=api.uidFromDocument(destnode.data);
				} else if (destnode.y + 1 == node.y && destnode.x == node.x) {
					destnode.linkedIn.bottom=api.uidFromDocument(node.data);
					node.linkedOut.top=api.uidFromDocument(destnode.data);
				} else if (destnode.y - 1 == node.y && destnode.x == node.x) {
					destnode.linkedIn.top=api.uidFromDocument(node.data);
					node.linkedOut.bottom=api.uidFromDocument(destnode.data);
				}
					
				//console.log("linkmode", destnode, node);
				that.drawNodeLinkInfo(node, $elem);
				that.drawNodeLinkInfo(destnode, $destelem);
				that.store();
				
				// actually don't do the overwrite
				return false;
			}
			rd.init('canvascontainer');
			
			$("#canvas td").on("dblclick.sceneitems", ".sceneItem", function() {
				var nodeindex = $(this).attr("data-nodeindex");
				if (!nodeindex) return;
				var data = that.nodes[nodeindex].data;
				$(that).trigger("itemtriggered", data);
			});
			
			$("#canvas td").on("click.sceneitems", ".btnExecute", function() {
				var nodeindex = $(this).attr("data-nodeindex");
				if (!nodeindex) return;
				var data = that.nodes[nodeindex].data;
				$(that).trigger("itemexecute", data);
			});
		},
		
		unload: function() {
			this.scene = null;
			this.nodes = [];
			this.currentLink = null;
			
			// unlink all event handlers of namespace sceneitems
			$(storageInstance).off(".sceneitems"); 
			
			// remove old elements
			$(".sceneItem").remove();
		},
 
		placeNode: function(node) {
			var $destelem;
			if (node.x==null || node.y==null) {
				$destelem = $("#canvas td:empty").first();
				
				if (!$destelem.length) {
					console.warn("No empty table cell found for node!", node);
					return;
				}
				node.x = $destelem.index();
				node.y = $destelem.parent().index();
			} else {
				$destelem = $("#canvas tbody").children().eq(node.y).children().eq(node.x);
			}
			//console.log("pos: "+node.text, node.x, node.y);
			return $destelem;
		},
 
		updateNodeLinkInfo: function(node, $destelem, updateAlsoOnSamePosition) {
			var newx = $destelem.index(), newy = $destelem.parent().index();
			if (!updateAlsoOnSamePosition && node.x == newx && node.y == newy)
				return;
			node.x = newx;
			node.y = newy;
			
			for (var i = 0; i <this.nodes.length; i++) {
				if (this.nodes[i].removeLinked(node.data))
					this.drawNodeLinkInfo(this.nodes[i], this.placeNode(this.nodes[i]));
			}
			
			node.removeAllLinks();
			this.drawNodeLinkInfo(node, $destelem);

			this.store();
		},
 
		drawNodeLinkInfo: function(node, $destelem) {
			if (!$destelem.children().length) {
				console.error("drawNodeLinkInfo: No valid destelem!", $destelem);
				$destelem.html("wrong");
				return;
			}
			
			// prepare canvas'
			var canvasDrawBG = document.createElement("canvas");
			canvasDrawBG.width = 200;
			canvasDrawBG.height = 120;
			var canvasDrawBGCtx = canvasDrawBG.getContext("2d");
			if (node.dropDestination) {
				canvasDrawBGCtx.rect(0, 0, canvasDrawBG.width, canvasDrawBG.height);
				canvasDrawBGCtx.fillStyle = 'yellow';
				canvasDrawBGCtx.fill();
			}
			canvasDrawBGCtx.strokeStyle = 'black';
			canvasDrawBGCtx.beginPath();
			
			// Draw top line. Two cases:
			// 1) Incoming link from top: Draw inside arc
			// 2) Outgoing link: Do not draw a line at all
			if (node.linkedIn.top) {
				canvasDrawBGCtx.moveTo(0, 0);
				canvasDrawBGCtx.lineTo(canvasDrawBG.width/2, 10);
				canvasDrawBGCtx.lineTo(canvasDrawBG.width, 0);
			} else if (!node.linkedOut.top) {
				canvasDrawBGCtx.moveTo(0, 0);
				canvasDrawBGCtx.lineTo(canvasDrawBG.width, 0);
			}
			
			// Draw bottom line
			if (node.linkedIn.bottom) {
				canvasDrawBGCtx.moveTo(0, canvasDrawBG.height);
				canvasDrawBGCtx.lineTo(canvasDrawBG.width/2, canvasDrawBG.height-10);
				canvasDrawBGCtx.lineTo(canvasDrawBG.width, canvasDrawBG.height);
			} else if (!node.linkedOut.bottom) {
				canvasDrawBGCtx.moveTo(0, canvasDrawBG.height);
				canvasDrawBGCtx.lineTo(canvasDrawBG.width, canvasDrawBG.height);
			}
			
			// Draw left line
			if (node.linkedIn.left) {
				canvasDrawBGCtx.moveTo(0, 0);
				canvasDrawBGCtx.lineTo(10, canvasDrawBG.height/2);
				canvasDrawBGCtx.lineTo(0, canvasDrawBG.height);
			} else if (!node.linkedOut.left) {
				canvasDrawBGCtx.moveTo(0, 0);
				canvasDrawBGCtx.lineTo(0, canvasDrawBG.height);
			}
			
			// Draw right line
			if (node.linkedIn.right) {
				canvasDrawBGCtx.moveTo(canvasDrawBG.width, 0);
				canvasDrawBGCtx.lineTo(canvasDrawBG.width-10, canvasDrawBG.height/2);
				canvasDrawBGCtx.lineTo(canvasDrawBG.width, canvasDrawBG.height);
			} else if (!node.linkedOut.right) {
				canvasDrawBGCtx.moveTo(canvasDrawBG.width, 0);
				canvasDrawBGCtx.lineTo(canvasDrawBG.width, canvasDrawBG.height);
			}
			
			canvasDrawBGCtx.stroke();
			$destelem.children().first().css("background-image", "url('"+ canvasDrawBG.toDataURL()+"')");
			
		},
 
		updateNode: function(node, $destelem) {
			if (!$destelem)
				$destelem = $("#canvas tbody").children().eq(node.y).children().eq(node.x);
			var bgclass = "btn-success";
			if (node.data.type_ == "condition") bgclass = "btn-info";
				else if (node.data.type_ == "action") bgclass = "btn-warning";																														 
			var entry = {title:node.data.method_,component:storageInstance.getComponentName(node.data.componentid_), type:node.data.type_, text:node.text, nodeindex: node.nodeindex, bgclass:bgclass};
			$destelem.html(this.ctx(entry));
			this.drawNodeLinkInfo(node, $destelem);
		},
 
		/**
		 * Call this method after insertion/removing of nodes
		 * and after node drag and drop.
		 */
		calcPredecessorNodes: function() {
			for (var sourceNodeIndex = 0; sourceNodeIndex < this.scene.v.length; sourceNodeIndex++) {
				var sceneNodeData = this.scene.v[sourceNodeIndex];
				if (sceneNodeData.e == null)
					continue;
				var sourceNode = this.nodes[sourceNodeIndex];
				
				var k = Object.keys(sceneNodeData.e);
				for (var i in k) { i = k[i];
					var nodeIndex = this.indexOfSceneItem(sceneNodeData.e[i]);
					if (nodeIndex==-1) {
						console.warn("Node not found", sceneNodeData.e[i]);
						continue;
					}
					var targetNode = this.nodes[nodeIndex];
					if (i=='top')
						targetNode.linkedIn.bottom = {type_:sceneNodeData.type_, id_:sceneNodeData.id_};
					else if (i=='bottom')
						targetNode.linkedIn.top = {type_:sceneNodeData.type_, id_:sceneNodeData.id_};
					else if (i=='left')
						targetNode.linkedIn.right = {type_:sceneNodeData.type_, id_:sceneNodeData.id_};
					else if (i=='right')
						targetNode.linkedIn.left = {type_:sceneNodeData.type_, id_:sceneNodeData.id_};
				}
			}
		},
 
		store: function() {
			this.scene.v = this.getGraph();
			//console.log("store", this.scene.v);
			websocketInstance.write(api.manipulatorAPI.update(this.scene));
		},

		getGraph: function() {
			var v = [];
			for (var i = 0; i < this.nodes.length; i++) {
				var node = this.nodes[i];
				var backupNode = {id_:node.data.id_, type_:node.data.type_, e:{}, editorprop:{x:node.x,y:node.y}};
				for (var l in node.linkedOut) {
					var linkedNode = node.linkedOut[l];
					if (!linkedNode)
						continue;
					backupNode.e[l] = linkedNode;
				}
				v.push(backupNode);
			}
			return v;
		}
	};
})(window);
