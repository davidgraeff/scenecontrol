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
		this.scene = null;
		this.nodes = [];
		this.links = [];
	}
	
	window.sceneNode = function(data) {
		this.that = this;
		this.incomingLinks = [];
		this.outgoingLinks = [];
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
	window.sceneLink = function(source, target) {
		this.that = this;
		this.source = source;
		this.target = target;
	}
	window.sceneNode.prototype = window.sceneLink.prototype = {
		enabled: true,
		setEnable: function(b) {
			enabled = b;
		},
		closestPointOnShapeX: function(node) {
			if (!this.width)
				return 0;

			var x = node.x + node.width/2;
			
			if (x < this.x) return this.x;
			else if (x > this.x + this.width) return this.x + this.width;
			else return x;
		},
		closestPointOnShapeY: function(node) {
			if (!this.height)
				return 0;

			var y = node.y + node.height/2;
			
			if (y < this.y) return this.y;
			else if (y > this.y + this.height) return this.y + this.height;
			else return y;
		}

	}
	
	window.sceneCanvas.prototype = {
		setCanvas: function(canvas) {
			this.canvas = canvas;
		},
		
		disable: function(sceneItemDocument) {
			for (var i = 0; i < this.nodes.length; i++) {
				if (this.nodes[i].data == sceneItemDocument) {
					this.nodes[i].setEnable(false);
				}
			}
			for (var i = 0; i < this.links.length; i++) {
				if (this.links[i].source.data == sceneItemDocument || this.links[i].target.data == sceneItemDocument) {
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
				if (this.links[i] == node || this.links[i].source == node || this.links[i].target == node) {
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
			node.setData(sceneItemDocument);
			node.setEnable(true);
		},

		load: function(scene) {
			// we need a destination object
			if (!this.canvas)
				return;
			
			this.unload();
			this.scene = scene;
			var that = this.that;
			var $canvas = $(this.canvas);

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
			
			$canvas.parent().on('resize.sceneitems', function() {
				that.force.size([$(that.canvas).width(), $(that.canvas).height()]).start();
				console.log("resize.p");
			});
			
			$canvas.on('resize.sceneitems', function() {
				that.force.size([$(that.canvas).width(), $(that.canvas).height()]).start();
				console.log("resize");
			});
			
			// variables
			var ctx = Handlebars.compile($("#scenegraphitem-template").html());
			var svg = d3.select(this.canvas);
// 			var color = d3.scale.category20b();
			var itemSizeWidth = 300, itemSizeHeight = 100,
				itemSizeWidthWPadding = itemSizeWidth + 50, itemSizeHeightWPadding = itemSizeHeight + 30;
			var width = $(this.canvas).width(),
				height = $(this.canvas).height(),
				sceneLength = scene.v.length,
				boundaryX = Math.floor(width/itemSizeWidthWPadding);

			// transform
			for (var i = 0; i < sceneLength; i++) {
				var sceneNodeData = scene.v[i];
				var sceneItem = storageInstance.getDocument(sceneNodeData.type_, sceneNodeData.id_);
				if (sceneItem==null) {
					console.warn("Cannot find scene item "+sceneNodeData.type_+ " " + sceneNodeData.id_);
					continue;
				}
				var node = new sceneNode(sceneItem);
				node.width = itemSizeWidth;
				node.height = itemSizeHeight;
				this.nodes.push(node);
			}
			
			for (var sourceNodeIndex = 0; sourceNodeIndex < scene.v.length; sourceNodeIndex++) {
				var sceneNodeData = scene.v[sourceNodeIndex];
				if (sceneNodeData.e == null)
					continue;
				var sourceNode = this.nodes[sourceNodeIndex];
				for (var targetNodeIndex = 0; targetNodeIndex < sceneNodeData.e.length; targetNodeIndex++) {
					var nodeIndex = this.indexOfSceneItem(sceneNodeData.e[targetNodeIndex]);
					if (nodeIndex==-1) {
						console.warn("Node not found", sceneNodeData.e[targetNodeIndex]);
						continue;
					}
					var targetNode = this.nodes[nodeIndex];
					var link = new sceneLink(sourceNode, targetNode);
					sourceNode.outgoingLinks.push(targetNodeIndex);
					targetNode.incomingLinks.push(sourceNodeIndex);
					this.links.push(link);
				}
			}
			
			// grid algorithm
			var maxx = 0, maxy = 0;

			function isConnectedTo(sourceNode, listOfUsedNodes) {
				for (var i=0;i<listOfUsedNodes.length;++i)
					if (sourceNode.incomingLinks.indexOf(listOfUsedNodes[i]) != -1 ||
						sourceNode.outgoingLinks.indexOf(listOfUsedNodes[i]) != -1)
						return true;
				return false;
			}
			function placeInGrid(grid, node, gridx, gridy) {
				if (gridx>maxx) maxx = gridx;
				if (gridy>maxy) maxy = gridy;
				grid[gridx][gridy] = node;
				node.gridx = gridx;
				node.gridy = gridy;
				console.log("place now @", gridx, gridy, node);
			}

			function createGridBase(grid, nodesOrdered) {
// 				var used = [];
				var usedIndeces = [];
				var gridx = 0;
				var i = 0;
				while(i<nodesOrdered.length) {
					var node = that.nodes[nodesOrdered[i]];
					if (!node) {
						console.error("node "+i, node, nodesOrdered[i]);
					}
					if (!isConnectedTo(node, usedIndeces)) {
						placeInGrid(grid, node, gridx, 0);
// 						used.push(node);
						usedIndeces.push(nodesOrdered[i]);
						++gridx;
						nodesOrdered.splice(i,1);
					} else {
						++i;
					}
				}
				return usedIndeces;
			}
			function reorderGridBase(grid, boundaryX, oversizedX) {
				var cells = getNextFreeGridCells(grid, 0,0, boundaryX, oversizedX+1-boundaryX);
				var i = 0;
				for (var cX=boundaryX;cX<oversizedX+1;++cX) {
					var npos = cells[i++];
					//console.log("move", cX, npos, grid[cX][0]);
					placeInGrid(grid, grid[cX][0], npos.x, npos.y);
					grid[cX][0] = null;
				}
				return Math.min(boundaryX-1, oversizedX);
			}
			function getNextFreeGridCells(grid, x,y, boundaryX, amountOfCells) {
				if (amountOfCells==0) return [];
				var cells = [];
				
				for (var cRadius = 0; cRadius < 5; cRadius++) {
					for (var cX=x-cRadius;cX<=x+cRadius;++cX) {
						if (cX<0 || cX >= boundaryX) continue;
						for (var cY=y-cRadius;cY<=y+cRadius;++cY) {
							if (cY<0 || grid[cX][cY] instanceof sceneNode || grid[cX][cY]==1) continue;
							grid[cX][cY]=1;
							cells.push({x:cX, y:cY});
// 							console.log("free grid cell", {x:cX, y:cY}, cells.length +" "+ amountOfCells);
							if (cells.length >= amountOfCells) return cells;
						}
					}
				}
				return cells;
			}
			
// 			function addToGrid(grid, nodeIndeces, boundaryX) {
// 				for (var i=0;i<nodeIndeces.length;++i) {
// 					var usedNode = that.nodes[nodeIndeces[i]];
// 					if (usedNode.outgoingLinks.length == 0) continue;
// 					console.log("free grid for links of", nodeIndeces[i], usedNode);
// 					var gridx = 0, gridy = 0;
// 					if (usedNode.incomingLinks.length) {
// 						var inode = that.nodes[usedNode.incomingLinks[0]];
// 						gridx = inode.gridx?inode.gridx:0;
// 						gridy = inode.gridy?inode.gridy:0;
// 					}
// 						
// 					var cells = getNextFreeGridCells(grid, gridx, gridy, boundaryX, usedNode.outgoingLinks.length);
// 					if (cells.length != usedNode.outgoingLinks.length) {
// 						console.error("No free grid cell found!", usedNode.outgoingLinks.length, usedNode);
// 						return; // failure!
// 					}
// 					for (var cCell=0;cCell<cells.length;++cCell) {
// 						placeInGrid(grid, that.nodes[usedNode.outgoingLinks[cCell]], cells[cCell].x, cells[cCell].y);
// 					}
// 					//console.log("loop:", i, nodeIndeces.length);
// 				}
// 			}
			function addToGrid(grid, nodeIndeces, boundaryX) {
				for (var i=0;i<nodeIndeces.length;++i) {
					var usedNode = that.nodes[nodeIndeces[i]];
					//if (usedNode.outgoingLinks.length == 0) continue;
					//console.log("free grid for links of", nodeIndeces[i], usedNode);
					var gridx = 0, gridy = 0;
					if (usedNode.incomingLinks.length) {
						var inode = that.nodes[usedNode.incomingLinks[0]];
						gridx = inode.gridx?inode.gridx:0;
						gridy = inode.gridy?inode.gridy:0;
					}
						
					var cells = getNextFreeGridCells(grid, gridx, gridy, boundaryX, 1);
					if (cells.length != 1) {
						console.error("No free grid cell found!", usedNode.outgoingLinks.length, usedNode);
						return; // failure!
					}
					
					for (var cCell=0;cCell<cells.length;++cCell) {
						placeInGrid(grid, that.nodes[nodeIndeces[i]], cells[cCell].x, cells[cCell].y);
					}
					//console.log("loop:", i, nodeIndeces.length);
				}
			}
			
			function Create2DArray(rows) {
				var arr = [];
				for (var i=0;i<rows;i++) arr[i] = [];
				return arr;
			}
			var grid = Create2DArray(100);
			
			// 1) Knoten ordnen nach eingehenden Kanten
			var nodesOrdered = [];
			for (var i = 0; i < sceneLength; i++) nodesOrdered.push(i);
 			nodesOrdered.sort(function(a,b) {return (that.nodes[a].incomingLinks.length>that.nodes[b].incomingLinks.length);});
			//  in erster grid row eintragen solange != nachbarn
			var used = createGridBase(grid, nodesOrdered);
			// loop) nachbarn um knoten positionieren
			//		a) edge contraint berücksichtigen
			//addNeighboursToGrid(grid, used, boundaryX);
			console.warn("used", used, nodesOrdered);
			addToGrid(grid, nodesOrdered, boundaryX);
			// 2) knoten in erster grid row in x richtung verschieben, so dass genau über evtl. nachbarn;
			//		a) edge contraint: an unbesetzten rändern positionieren statt > boundaryX
			maxx = reorderGridBase(grid, boundaryX, maxx);
			
			// calculate center positioning
			var rowoffsetx = (width - itemSizeWidthWPadding*(maxx+1)) / 2;
			var rowoffsety = (height - itemSizeHeightWPadding*(maxy+1)) / 2;
			if (rowoffsety< 10) rowoffsety = 10;
			

			// set node parameters
			for (var x=0;x<maxx+1;++x) {
				for (var y=0;y<maxy+1;++y) {
					var obj = grid[x][y];
					if (obj instanceof sceneNode) {
						obj.x = rowoffsetx + x*itemSizeWidthWPadding;
						obj.y = rowoffsety + y*itemSizeHeightWPadding;
						console.log("obj",x+","+y,rowoffsetx+","+rowoffsety);
					}
				}
			}

			// draw
			this.visnode = svg.selectAll(".node")
				.data(this.nodes)
				.enter().append("foreignObject")
				.attr("class", "node").attr("width", function(d,i){return d.width}).attr("height", function(d,i){return d.height})
				.attr("x", function(d,i){return d.x}).attr("y", function(d,i){return d.y})
				/*.call(this.force.drag)*/;

			this.vislink = svg.selectAll(".link")
				.data(this.links)
				.enter().append("line")
				.attr("class", "link").attr("marker-end", "url(#pf1)")
				.attr("x1", function(d,i){return d.source.closestPointOnShapeX(d.target)}).attr("y1", function(d,i){return d.source.closestPointOnShapeY(d.target)})
				.attr("x2", function(d,i){return d.target.closestPointOnShapeX(d.source)}).attr("y2", function(d,i){return d.target.closestPointOnShapeY(d.source)});
				
			this.visnode.append("xhtml:body").attr('xmlns','http://www.w3.org/1999/xhtml').attr("style","height:"+itemSizeHeight+"px;").html(function(d, i) {
					var bgclass = "btn-success";
					if (d.data.type_ == "condition") bgclass = "btn-info";
					else if (d.data.type_ == "action") bgclass = "btn-warning";																														 
					var entry = {title:d.data.method_,component:d.data.componentid_, type:d.data.type_, text:d.text, nodeindex: i, bgclass:bgclass};
					return ctx(entry);
				});
			
			$(".sceneItemMain").on("dblclick.sceneitems", function() {
				var nodeindex = $(this).attr("data-nodeindex");
				if (!nodeindex) return;
				var data = that.nodes[nodeindex].data;
				$(that).trigger("itemtriggered", data);
			});
			
		},
		
		unload: function() {
			this.scene = null;
			this.nodes = [];
			this.links = [];
			this.currentLink = null;
			
			// unlink all event handlers of namespace sceneitems
			$(storageInstance).off(".sceneitems"); 
			$(this.canvas).off(".sceneitems"); 
			//$(document).off(".sceneitems"); 
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
				
				var backupNode = {id_:node.data.id_, type_:node.data.type_, e:[]};
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
