/**
 * Scene Canvas Module - Contains all nodes, links objects. Create nodes/links from scene document.
 * Available as window.sceneCanvas
 * Uses modules: Canvas
 */
(function (window) {
	"use strict";
	window.sceneCanvas = function() {
		this.that = this;
		this.canvas = null;
		this.ctx = null;
		this.scene = null;
		this.nodes = [];
		this.links = [];
		
		this.shift= null;
		this.originalMouse= null;
		this.scrollMaxX= null;
		this.scrollMaxY= null;
		this.movingObject =  null;
		this.selectedObject = null; // either a Link or a Node
		this.currentLink = null; // a Link
	}
	
	window.sceneCanvas.prototype = {
		setCanvas: function(canvas) {
			this.canvas = canvas;
			this.ctx = canvas.getContext('2d');
		},
		
		draw: function() {
			var c = this.ctx;
			c.clearRect(0, 0, this.canvas.width, this.canvas.height);
			c.save();
			c.translate(0.5, 0.5);
			
			for (var i = 0; i < this.nodes.length; i++) {
				c.save();
				c.strokeStyle = 'gray';
				if (!this.nodes[i].enabled) {
					c.globalAlpha = 0.4;
				} else if (this.nodes[i] == this.selectedObject) {
					c.strokeStyle = 'blue';
				}
				//c.fillStyle = (this == this.selectedObject) ? 'blue' : 'black';
				this.nodes[i].draw(c);
				c.restore();
			}
			for (var i = 0; i < this.links.length; i++) {
				c.save();
				c.lineWidth = 1;
				if (!this.links[i].enabled)
					c.globalAlpha = 0.4;
				c.fillStyle = c.strokeStyle = (this.links[i] == this.selectedObject) ? 'blue' : 'black';
				this.links[i].draw(c);
				c.restore();
			}
			if (this.currentLink != null) {
				c.save();
				c.lineWidth = 1;
				c.fillStyle = c.strokeStyle = 'black';
				this.currentLink.draw(c);
				c.restore();
			}
			
			c.restore();
		},
		
		selectObject: function(x, y) {
			for (var i = 0; i < this.nodes.length; i++) {
				if (this.nodes[i].containsPoint(x, y)) {
					return this.nodes[i];
				}
			}
			for (var i = 0; i < this.links.length; i++) {
				if (this.links[i].containsPoint(x, y)) {
					return this.links[i];
				}
			}
			return null;
		},
 
		disable: function(sceneItemDocument) {
			for (var i = 0; i < this.nodes.length; i++) {
				if (this.nodes[i].data == sceneItemDocument) {
					this.nodes[i].setEnable(false);
				}
			}
			for (var i = 0; i < this.links.length; i++) {
				if (this.links[i].nodeA.data == sceneItemDocument || this.links[i].nodeB.data == sceneItemDocument) {
					this.links[i].setEnable(false);
				}
			}
		},

		removeNode: function(node) {
			for (var i = 0; i < this.nodes.length; i++) {
				if (this.nodes[i] == node) {
					this.nodes.splice(i--, 1);
				}
			}
			for (var i = 0; i < this.links.length; i++) {
				if (this.links[i] == node || this.links[i].node == node || this.links[i].nodeA == node || this.links[i].nodeB == node) {
					this.links.splice(i--, 1);
				}
			}
		},
 
		removeLink: function(link) {
			for (var i = 0; i < this.links.length; i++) {
				if (this.links[i] == link) {
					this.links.splice(i--, 1);
				}
			}
		},
		
		moveNodes: function(x, y) {
			for (var i = 0; i < this.nodes.length; i++) {
				this.nodes[i].x += x;
				this.nodes[i].y += y;
			}
		},
 
		snapNode: function() {
			if (this.selectedObject instanceof Node) {
				for (var i = 0; i < this.nodes.length; i++) {
					this.nodes[i].snap(this.selectedObject);
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
 
		sceneitemchanged: function(sceneItemDocument, temp) {
			var i = this.indexOfSceneItem(sceneItemDocument);
			if (i==-1) {
				return;
			}
			var node = this.nodes[i];
			node.setData(this.ctx, sceneItemDocument);
			node.setEnable(true);
			this.draw();
		},
 
		clear: function() {
			this.nodes = [];
			this.links = [];
			this.currentLink = null;
			
			this.draw();
		},
 
		load: function(scene) {
			// we need a destination object
			if (!this.canvas)
				return;
			
			this.unload();
			this.scene = scene;
			var that = this.that;

			// connect event handlers
			$(storageInstance).on('onevent.sceneitems', function(d, flags) {
				if (flags.doc.sceneid_ != sceneCanvas.scene.id_)
					return;
				
				if (!flags.removed)
					sceneCanvas.sceneitemchanged(flags.doc, flags.temporary);
			});

			$(storageInstance).on('oncondition.sceneitems', function(d, flags) {
				if (flags.doc.sceneid_ != sceneCanvas.scene.id_)
					return;
				
				if (!flags.removed)
					sceneCanvas.sceneitemchanged(flags.doc, flags.temporary);
			});

			$(storageInstance).on('onaction.sceneitems', function(d, flags) {
				if (flags.doc.sceneid_ != sceneCanvas.scene.id_)
					return;
				
				if (!flags.removed)
					sceneCanvas.sceneitemchanged(flags.doc, flags.temporary);
			});
			
			// connect event handlers
			$(this.canvas).on('mousedown.sceneitems', function(e) {
				if (e.which!=1)
					return;
				
				// set focus to canvas (to get key strokes)
				$(that.canvas).attr("tabindex", "0");
				// mouse position
				var mouse = {'x': e.pageX - $(that.canvas).offset().left,'y': e.pageY - $(that.canvas).offset().top};

				that.selectedObject = that.selectObject(mouse.x, mouse.y);
				
				that.movingObject = false;
				if (that.selectedObject != null) {
					if (that.shift && that.selectedObject instanceof Node) {
						$(that.canvas).css('cursor', 'pointer');
						that.currentLink = new TemporaryLink(that.selectedObject, mouse);
					} else {
						$(that.canvas).css('cursor', 'move');
						that.movingObject = true;
						if (that.selectedObject.setMouseStart) {
							that.selectedObject.setMouseStart(mouse.x, mouse.y);
						}
					}
				} else {
					that.originalMouse = mouse;
					$(that.canvas).css('cursor', 'move');
				}
				that.draw();
				
				if (document.activeElement == canvas) {
					// disable drag-and-drop only if the canvas is already focused
					return false;
				} else {
					// otherwise, let the browser switch the focus away from wherever it was
					return true;
				}
			});
			
			this.canvas.ondblclick = function(e) {
				var mouse = {'x': e.pageX - $(that.canvas).offset().left,'y': e.pageY - $(that.canvas).offset().top};
				that.selectedObject = that.selectObject(mouse.x, mouse.y);
				
				if (that.selectedObject == null)
					return;
				
				if (that.selectedObject instanceof Node && that.selectedObject.enabled) {
					SceneItemsUIHelper.showSceneItemDialog(that.selectedObject.data, false);
				} else if (that.selectedObject instanceof Link) {
					that.removeLink(that.selectedObject);
					that.draw();
					that.store();
				}
			};
			
			$(this.canvas).on('mousemove.sceneitems', function(e) {
				if (that.currentLink != null) {
					var mouse = {'x': e.pageX - $(that.canvas).offset().left,'y': e.pageY - $(that.canvas).offset().top}; 
					var targetNode = that.selectObject(mouse.x, mouse.y);
					if (!(targetNode instanceof Node)) {
						targetNode = null;
					}
					
					if (that.selectedObject != null) {
						if (targetNode != null) {
							that.currentLink = new Link(that.selectedObject, targetNode);
						} else {
							that.currentLink = new TemporaryLink(that.selectedObject.closestPointOnShape(mouse.x, mouse.y), mouse);
						}
					}
					that.draw();
				} else if (that.movingObject) {
					that.selectedObject.setAnchorPoint(e.pageX - $(that.canvas).offset().left, e.pageY - $(that.canvas).offset().top);
					that.snapNode();
					that.draw();
				} else if (that.originalMouse != null) { // move nodes
					var mouse = {'x': e.pageX - $(that.canvas).offset().left,'y': e.pageY - $(that.canvas).offset().top}; 
					that.moveNodes(mouse.x - originalMouse.x,
											mouse.y - originalMouse.y);
					that.draw();
					that.originalMouse = mouse;
				}
			});
			
			$(this.canvas).on('mouseout.sceneitems', function () {
				if (that.movingObject)
					that.store();
				that.currentLink = null;
				that.draw();
				that.movingObject = false;
				that.originalMouse = null;
				$(that.canvas).css('cursor', 'default');
			});
			
			$(document).on('mouseup.sceneitems', function(e) {
				if (that.currentLink != null) {
					if (!(that.currentLink instanceof TemporaryLink)) {
						that.selectedObject = that.currentLink;
						that.links.push(that.currentLink);
						that.movingObject = true; // reuse as indication for that.store()
					}
					that.currentLink = null;
					that.draw();
				}
				if (that.movingObject || that.originalMouse)
					that.store();
				
				that.movingObject = false;
				that.originalMouse = null;
				$(that.canvas).css('cursor', 'default');
			});
			
			$(this.canvas).keydown(function(event) {
				var key = event.which;
				if (key == 16) {
					that.shift = true;
				}
			});
			
			$(this.canvas).keyup(function(event) {
				var key = event.which;
				
				if (key == 16) {
					that.shift = false;
				}
			});

			
			//var sceneDocuments = storageInstance.documentsForScene(sceneid);
			// 	console.log("load scene", scene);
			var padding = 50;
			var startx = 0; var starty = 0;

			for (var i = 0; i < scene.v.length; i++) {
				var sceneNode = scene.v[i];
				var sceneItem = storageInstance.getDocument(sceneNode.type_, sceneNode.id_);
				if (sceneItem==null) {
					console.warn("Cannot find scene item "+sceneNode.type_+ " " + sceneNode.id_);
					continue;
				}
				var node = new Node();
				node.setData(this.ctx, sceneItem);
				this.nodes.push(node);
				
				if (sceneNode.draw_ == null) {
					if (i==0) {
						startx += node.width;
						starty += node.height;
					}
					if (startx>this.canvas.width) {
						startx = node.width;
						starty += node.height+padding;
					}
					sceneNode.draw_ = {'x':startx,'y':starty};
					startx += node.width+padding;
				}
				
				node.setPos(sceneNode.draw_.x, sceneNode.draw_.y);
			}
			
			for (var i = 0; i < scene.v.length; i++) {
				var sceneNode = scene.v[i];
				if (sceneNode.e == null)
					continue;
				for (var l = 0; l < sceneNode.e.length; l++) {
					var nodeIndex = this.indexOfSceneItem(sceneNode.e[l]);
					if (nodeIndex==-1) {
						console.warn("Node not found", sceneNode.e[l]);
						continue;
					}
					var link = new Link(this.nodes[i], this.nodes[nodeIndex]);
					this.links.push(link);
				}
			}
			
			this.draw();
		},
		
		unload: function() {
			this.scene = null;
			this.nodes = [];
			this.links = [];
			this.currentLink = null;
			
			// unlink all event handlers of namespace sceneitems
			$(storageInstance).off(".sceneitems"); 
			$(this.canvas).off(".sceneitems"); 
			$(document).off(".sceneitems"); 
		},
		
		store: function() {
			this.scene.v = this.getGraph();
			websocketInstance.updateDocument(this.scene);
		},
		
		getImage: function(format) { // 'image/jpeg'
			return this.canvas.toDataURL(((format==undefined)?"image/png":format));
		},
		
		getGraph: function() {
			var v = [];
			for (var i = 0; i < this.nodes.length; i++) {
				var node = this.nodes[i];
				
				var backupNode = {id_:node.data.id_, type_:node.data.type_, draw_:{'x': node.x,'y': node.y}, e:[]};
				for (var l = 0; l < this.links.length; l++) {
					var link = this.links[l];
					if (link.nodeA!=this.nodes[i])
						continue;
					var otherNode = this.nodes.indexOf(link.nodeB);
					if (otherNode == -1)
						continue;
					otherNode = this.nodes[otherNode];

					var backupLink = {id_:otherNode.data.id_, type_:otherNode.data.type_};
					backupNode.e.push(backupLink);
				}
				v.push(backupNode);
			}
			return v;
		}
	};
})(window);
