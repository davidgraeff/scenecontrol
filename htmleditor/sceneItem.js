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
		nodes: [],
		links: [],
		
		selectedObject: null, // either a Link or a Node
		currentLink: null, // a Link
		
		draw: function(canvas) {
			var c = canvas.getContext('2d');
			c.clearRect(0, 0, canvas.width, canvas.height);
			c.save();
			
			for (var i = 0; i < this.nodes.length; i++) {
				c.strokeStyle = (this.nodes[i] == this.selectedObject) ? 'blue' : 'gray';
				//c.fillStyle = (this == this.selectedObject) ? 'blue' : 'black';
				this.nodes[i].draw(c);
			}
			for (var i = 0; i < this.links.length; i++) {
				c.lineWidth = 1;
				c.fillStyle = c.strokeStyle = (this.links[i] == this.selectedObject) ? 'blue' : 'black';
				this.links[i].draw(c);
			}
			if (this.currentLink != null) {
				c.lineWidth = 1;
				c.fillStyle = c.strokeStyle = 'black';
				this.currentLink.draw(c);
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
 
		removeSelected: function() {
			for (var i = 0; i < this.nodes.length; i++) {
				if (this.nodes[i] == this.selectedObject) {
					this.nodes.splice(i--, 1);
				}
			}
			for (var i = 0; i < this.links.length; i++) {
				if (this.links[i] == this.selectedObject || this.links[i].node == this.selectedObject || this.links[i].nodeA == this.selectedObject || links[i].nodeB == this.selectedObject) {
					this.links.splice(i--, 1);
				}
			}
			this.selectedObject = null;
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
					this.nodes[i].snap(selectedObject);
				}
			}
		},

		indexOfSceneItem: function(sceneItem) {
			for (var i = 0; i < this.nodes.length; i++) {
				var node = this.nodes[i].data;
				if (node.type_==sceneItem.type_ && node.id_==sceneItem.id_)
					return i;
			}
			return -1;
		},
		
		load: function(scene, maxX, canvasContext) {
			this.nodes = [];
			this.links = [];
			
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
				node.setData(canvasContext, sceneItem);
				this.nodes.push(node);
				
				if (sceneNode.draw_ == null) {
					if (i==0) {
						startx += node.width;
						starty += node.height;
					}
					if (startx>maxX) {
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
		}
	};

})(window);
