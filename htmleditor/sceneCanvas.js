/**
 * Scene Canvas Module - Contains all nodes, links objects. Create nodes/links from scene document.
 * Available as window.sceneCanvas
 * Uses modules: Canvas
 */
(function (window) {
	"use strict";
	window.sceneCanvas = {
		/*
		* Based on Finite State Machine Designer (http://madebyevan.com/fsm/)
		* License: MIT License, Finite State Machine Designer Copyright (c) 2010 Evan Wallace
		*/
		canvas: null,
		ctx: null,
		nodes: [],
		links: [],
		
		selectedObject: null, // either a Link or a Node
		currentLink: null, // a Link
		bgPattern: null,
 
		setCanvas: function(canvas) {
			this.canvas = canvas;
			this.ctx = canvas.getContext('2d');
			var image = new Image();
			image.src = "brushed-silver-metallic-background.jpg";
			this.bgPattern = this.ctx.createPattern(image, "repeat");
		},
		
		draw: function() {
			var c = this.ctx;
			c.save();
			if (this.bgPattern==null)
				c.clearRect(0, 0, this.canvas.width, this.canvas.height);
			else {
				c.fillStyle = this.bgPattern;
				c.fillRect(0, 0, this.canvas.width, this.canvas.height);
			}
			c.restore();
			c.save();
			c.translate(0.5, 0.5)
			
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
				var node = this.nodes[i].data;
				if (node.type_==sceneItemDocument.type_ && node.id_==sceneItemDocument.id_)
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
			this.nodes = [];
			this.links = [];
			this.currentLink = null;
			
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
		
		getGraph: function() {
			var backup = { 'v': [] };
			for (var i = 0; i < this.nodes.length; i++) {
				var node = this.nodes[i];
				
				var backupNode = {id_:node.data.id_, type_:node.data.type_, draw_:{'x': node.x,'y': node.y}, e:[]};
				for (var l = 0; l < this.links.length; l++) {
					var link = this.links[l];
					if (link.nodeA!=this.nodes[i])
						continue;
					var otherNode = this.nodes.indexOf(link.nodeB);
					if (otherNode == null)
						continue;
					var backupLink = {id_:otherNode.data.id_, type_:otherNode.data.type_};
					backupNode.e.push(backupLink);
				}
				backup.v.push(backupNode);
			}
			return backup;
		}
	};

	// unlink all event handlers of namespace sceneitems
	$(document).off(".sceneitems"); 
	// connect event handlers
	$(storageInstance).on('onevent.sceneitems', function(d, flags) {
		if (flags.doc.sceneid_ != CurrentScene.id)
			return;
		
		if (!flags.removed)
			sceneCanvas.sceneitemchanged(flags.doc, flags.temporary);
	});

	$(storageInstance).on('oncondition.sceneitems', function(d, flags) {
		if (flags.doc.sceneid_ != CurrentScene.id)
			return;
		
		if (!flags.removed)
			sceneCanvas.sceneitemchanged(flags.doc, flags.temporary);
	});

	$(storageInstance).on('onaction.sceneitems', function(d, flags) {
		if (flags.doc.sceneid_ != CurrentScene.id)
			return;
		
		if (!flags.removed)
			sceneCanvas.sceneitemchanged(flags.doc, flags.temporary);
	});
})(window);
