/*
 * Based on Finite State Machine Designer (http://madebyevan.com/fsm/)
 * License: MIT License, Finite State Machine Designer Copyright (c) 2010 Evan Wallace
 */

var canvas; var $canvas;
var nodeRadius = 30;
var nodes = [];
var links = [];

var snapToPadding = 6; // pixels
var hitTargetPadding = 6; // pixels
var selectedObject = null; // either a Link or a Node
var currentLink = null; // a Link
var movingObject = false;
var shift = false;
var originalMouse = null;
var scrollMaxX = 0; var scrollMaxY = 0;

CanvasRenderingContext2D.prototype.roundRect = function (x, y, w, h, r) {
	if (w < 2 * r) r = w / 2;
	if (h < 2 * r) r = h / 2;
	this.beginPath();
	this.moveTo(x+r, y);
	this.arcTo(x+w, y,   x+w, y+h, r);
	this.arcTo(x+w, y+h, x,   y+h, r);
	this.arcTo(x,   y+h, x,   y,   r);
	this.arcTo(x,   y,   x+w, y,   r);
	this.closePath();
	return this;
}

function Link(a, b) {
	this.nodeA = a;
	this.nodeB = b;

	// make anchor point relative to the locations of nodeA and nodeB
 	this.parallelPart = 0.5; // percentage from nodeA to nodeB
 	this.perpendicularPart = 0; // pixels from line between nodeA and nodeB
}

Link.prototype.getAnchorPoint = function() {
	var dx = this.nodeB.x - this.nodeA.x;
	var dy = this.nodeB.y - this.nodeA.y;
	var scale = Math.sqrt(dx * dx + dy * dy);
	return {
		'x': this.nodeA.x + dx * this.parallelPart - dy * this.perpendicularPart / scale,
		'y': this.nodeA.y + dy * this.parallelPart + dx * this.perpendicularPart / scale
	};
};

Link.prototype.setAnchorPoint = function(x, y) {
	var dx = this.nodeB.x - this.nodeA.x;
	var dy = this.nodeB.y - this.nodeA.y;
	var scale = Math.sqrt(dx * dx + dy * dy);
	this.parallelPart = (dx * (x - this.nodeA.x) + dy * (y - this.nodeA.y)) / (scale * scale);
// 	this.perpendicularPart = (dx * (y - this.nodeA.y) - dy * (x - this.nodeA.x)) / scale;
	this.perpendicularPart = 0; // force straight line
	// snap to a straight line
// 	if (this.parallelPart > 0 && this.parallelPart < 1 && Math.abs(this.perpendicularPart) < snapToPadding) {
// 		this.perpendicularPart = 0;
// 	}
};

Link.prototype.getEndPointsAndCircle = function() {
	if (this.perpendicularPart == 0) {
		var midX = (this.nodeA.x + this.nodeB.x) / 2;
		var midY = (this.nodeA.y + this.nodeB.y) / 2;
		var start = this.nodeA.closestPointOnShape(midX, midY);
		var end = this.nodeB.closestPointOnShape(midX, midY);
		return {
			'hasCircle': false,
			'startX': start.x,
			'startY': start.y,
			'endX': end.x,
			'endY': end.y
		};
	}
	var anchor = this.getAnchorPoint();
	var circle = circleFromThreePoints(this.nodeA.x, this.nodeA.y, this.nodeB.x, this.nodeB.y, anchor.x, anchor.y);
	var isReversed = (this.perpendicularPart > 0);
	var reverseScale = isReversed ? 1 : -1;
	var startAngle = Math.atan2(this.nodeA.y - circle.y, this.nodeA.x - circle.x) - reverseScale * nodeRadius / circle.radius;
	var endAngle = Math.atan2(this.nodeB.y - circle.y, this.nodeB.x - circle.x) + reverseScale * nodeRadius / circle.radius;
	var startX = circle.x + circle.radius * Math.cos(startAngle);
	var startY = circle.y + circle.radius * Math.sin(startAngle);
	var endX = circle.x + circle.radius * Math.cos(endAngle);
	var endY = circle.y + circle.radius * Math.sin(endAngle);
	return {
		'hasCircle': true,
		'startX': startX,
		'startY': startY,
		'endX': endX,
		'endY': endY,
		'startAngle': startAngle,
		'endAngle': endAngle,
		'circleX': circle.x,
		'circleY': circle.y,
		'circleRadius': circle.radius,
		'reverseScale': reverseScale,
		'isReversed': isReversed
	};
};

Link.prototype.draw = function(c) {
	var stuff = this.getEndPointsAndCircle();
	// draw arc
	c.beginPath();
	if (stuff.hasCircle) {
		c.arc(stuff.circleX, stuff.circleY, stuff.circleRadius, stuff.startAngle, stuff.endAngle, stuff.isReversed);
	} else {
		c.moveTo(stuff.startX, stuff.startY);
		c.lineTo(stuff.endX, stuff.endY);
	}
	c.stroke();
	// draw the head of the arrow
	if (stuff.hasCircle) {
		drawArrow(c, stuff.endX, stuff.endY, stuff.endAngle - stuff.reverseScale * (Math.PI / 2));
	} else {
		drawArrow(c, stuff.endX, stuff.endY, Math.atan2(stuff.endY - stuff.startY, stuff.endX - stuff.startX));
	}
};

Link.prototype.containsPoint = function(x, y) {
	var stuff = this.getEndPointsAndCircle();
	if (stuff.hasCircle) {
		var dx = x - stuff.circleX;
		var dy = y - stuff.circleY;
		var distance = Math.sqrt(dx*dx + dy*dy) - stuff.circleRadius;
		if (Math.abs(distance) < hitTargetPadding) {
			var angle = Math.atan2(dy, dx);
			var startAngle = stuff.startAngle;
			var endAngle = stuff.endAngle;
			if (stuff.isReversed) {
				var temp = startAngle;
				startAngle = endAngle;
				endAngle = temp;
			}
			if (endAngle < startAngle) {
				endAngle += Math.PI * 2;
			}
			if (angle < startAngle) {
				angle += Math.PI * 2;
			} else if (angle > endAngle) {
				angle -= Math.PI * 2;
			}
			return (angle > startAngle && angle < endAngle);
		}
	} else {
		var dx = stuff.endX - stuff.startX;
		var dy = stuff.endY - stuff.startY;
		var length = Math.sqrt(dx*dx + dy*dy);
		var percent = (dx * (x - stuff.startX) + dy * (y - stuff.startY)) / (length * length);
		var distance = (dx * (y - stuff.startY) - dy * (x - stuff.startX)) / length;
		return (percent > 0 && percent < 1 && Math.abs(distance) < hitTargetPadding);
	}
	return false;
};

function Node() {
	this.mouseOffsetX = 0;
	this.mouseOffsetY = 0;
	this.fontsize = 20;
	this.width = 0;
	this.maxwidth = 200;
	this.height = 0;
	this.textPadding = 10;
	this.headerHeight = 25 + 2 * this.textPadding;
	this.cachetext = [];
}

Node.prototype.setPos = function(x, y) {
	this.x = x;
	this.y = y;
};

Node.prototype.setMouseStart = function(x, y) {
	this.mouseOffsetX = this.x - x;
	this.mouseOffsetY = this.y - y;
};

Node.prototype.setAnchorPoint = function(x, y) {
	this.x = x + this.mouseOffsetX;
	this.y = y + this.mouseOffsetY;
};

/**
 * Canvas context: c
 * Data: d
 */
Node.prototype.setData = function(c, d) {
	c.font = this.fontsize+'px "Times New Roman", serif';
	
	this.data = d;
	this.dataschema = storageInstance.schemaForDocument(d);
	var text;
	if (this.dataschema) {
		if (this.dataschema.text) {
			text = sceneItemText( this.data, this.dataschema );
		} else {
			text = this.dataschema.name;
		}
	} else {
		// No schema: We do not know how to express this item in text
		text = JSON.stringify(this.data);
	}
	
	// determine lines
	this.cachetext = fragmentText(c, text, this.maxwidth);
	
	// determine max width
	var maxW = 0;
	for (var i=0;i<this.cachetext.length;++i) {
		var w = c.measureText(this.cachetext[i]).width + 2*this.textPadding;
		if (w>maxW)
			maxW = w;
	};
	this.width = maxW;
	// determine height
	this.height = this.cachetext.length * this.fontsize + 2*this.textPadding + this.headerHeight;
	
};

Node.prototype.draw = function(c) {
	c.save();
	c.font = this.fontsize+'px "Times New Roman", serif';
	var w2 = this.width/2;
	var h2 = this.height/2;
		
	// draw main rectangle
	c.lineWidth = 2;
	c.strokeStyle = (this == selectedObject) ? 'blue' : 'gray';
	//c.fillStyle = (this == selectedObject) ? 'blue' : 'black';
	c.roundRect(this.x-w2, this.y-h2, this.width, this.height, 10);
	c.stroke();
	
	{ // shadow
		c.save();
		c.fillStyle = 'white';
		c.shadowColor = '#999';
		c.shadowBlur = 10;
		c.shadowOffsetX = 3;
		c.shadowOffsetY = 3;
		c.fill();
		c.restore();
	}
	
	// clip to the outer rectangle
	c.clip();
	
	{ // gradient
		c.save();
		var lingrad = c.createLinearGradient(0, 0, 0, this.headerHeight);
		lingrad.addColorStop(0, '#F0F0F0');
		lingrad.addColorStop(1, '#DDD');
		c.fillStyle = lingrad;
		c.fillRect(this.x-w2, this.y-h2, this.width, this.headerHeight);
// 		c.stroke();
		c.restore();
	}

	// draw outer line again
	c.stroke();
	
	{ // Draw title
		c.fillStyle = 'black';
		c.fillText(this.data.type_, this.x-w2+this.textPadding, this.y-h2+this.headerHeight/2);
	}
	
	{
		// draw main text
		var x = this.x - w2 + this.textPadding;
		var y = this.y + this.headerHeight - h2;
		
		x = Math.round(x);
		y = Math.round(y);
		c.fillStyle = 'black';
		for (var i=0;i<this.cachetext.length;++i) {
			c.fillText(this.cachetext[i], x, y + (i + 1) * this.fontsize);
		};
	}
	
	c.restore();
};

Node.prototype.closestPointOnShape = function(x, y) {
	if (!this.width)
		return {};
// 	var dx = x - this.x;
// 	var dy = y - this.y;
// 	var scale = Math.sqrt(dx * dx + dy * dy);
// 	return {
// 		'x': this.x + dx * nodeRadius / scale,
// 		'y': this.y + dy * nodeRadius / scale
// 	};
		var w2 = this.width/2;
		var dx;
		if (x<this.x-w2) dx = this.x-w2;
		else if (x>this.x+w2) dx = this.x+w2;
		else dx = x;
			
		var h2 = this.height/2;
		var dy;
		if (y<this.y-h2) dy = this.y-h2;
		else if (y>this.y+h2) dy = this.y+h2;
		else dy = y;
		
		return {
			'x': dx,
			'y': dy
		};
};

Node.prototype.containsPoint = function(x, y) {
	if (!this.width)
		return false;
	// 	return (x - this.x)*(x - this.x) + (y - this.y)*(y - this.y) < nodeRadius*nodeRadius;
		var w2 = this.width/2;
		var h2 = this.height/2;
		return (x>=this.x-w2 && x<=this.x+w2 && y>=this.y-h2 && y<=this.y+h2);
};

function TemporaryLink(from, to) {
	this.from = from;
	this.to = to;
}

TemporaryLink.prototype.draw = function(c) {
	// draw the line
	c.beginPath();
	c.moveTo(this.to.x, this.to.y);
	c.lineTo(this.from.x, this.from.y);
	c.stroke();
	
	// draw the head of the arrow
	drawArrow(c, this.to.x, this.to.y, Math.atan2(this.to.y - this.from.y, this.to.x - this.from.x));
};

function drawArrow(c, x, y, angle) {
	var dx = Math.cos(angle);
	var dy = Math.sin(angle);
	c.beginPath();
	c.moveTo(x, y);
	c.lineTo(x - 8 * dx + 5 * dy, y - 8 * dy - 5 * dx);
	c.lineTo(x - 8 * dx - 5 * dy, y - 8 * dy + 5 * dx);
	c.fill();
}

function det(a, b, c, d, e, f, g, h, i) {
	return a*e*i + b*f*g + c*d*h - a*f*h - b*d*i - c*e*g;
}

function circleFromThreePoints(x1, y1, x2, y2, x3, y3) {
	var a = det(x1, y1, 1, x2, y2, 1, x3, y3, 1);
	var bx = -det(x1*x1 + y1*y1, y1, 1, x2*x2 + y2*y2, y2, 1, x3*x3 + y3*y3, y3, 1);
	var by = det(x1*x1 + y1*y1, x1, 1, x2*x2 + y2*y2, x2, 1, x3*x3 + y3*y3, x3, 1);
	var c = -det(x1*x1 + y1*y1, x1, y1, x2*x2 + y2*y2, x2, y2, x3*x3 + y3*y3, x3, y3);
	return {
		'x': -bx / (2*a),
		'y': -by / (2*a),
		'radius': Math.sqrt(bx*bx + by*by - 4*a*c) / (2*Math.abs(a))
	};
}

function fragmentText(ctx, text, maxWidth) {
	var words = text.split(' '),
		lines = [],
	line = "";
	if (ctx.measureText(text).width < maxWidth) {
		return [text];
	}
	while (words.length > 0) {
		while (ctx.measureText(words[0]).width >= maxWidth) {
			var tmp = words[0];
			words[0] = tmp.slice(0, -1);
			if (words.length > 1) {
				words[1] = tmp.slice(-1) + words[1];
			} else {
				words.push(tmp.slice(-1));
			}
		}
		if (ctx.measureText(line + words[0]).width < maxWidth) {
			line += words.shift() + " ";
		} else {
			lines.push(line);
			line = "";
		}
		if (words.length === 0) {
			lines.push(line);
		}
	}
	return lines;
}


function draw() {
	var c = canvas.getContext('2d');
	c.clearRect(0, 0, canvas.width, canvas.height);
	c.save();

	for (var i = 0; i < nodes.length; i++) {
		nodes[i].draw(c);
	}
	for (var i = 0; i < links.length; i++) {
		c.lineWidth = 1;
		c.fillStyle = c.strokeStyle = (links[i] == selectedObject) ? 'blue' : 'black';
		links[i].draw(c);
	}
	if (currentLink != null) {
		c.lineWidth = 1;
		c.fillStyle = c.strokeStyle = 'black';
		currentLink.draw(c);
	}
	
	c.restore();
}

function selectObject(x, y) {
	for (var i = 0; i < nodes.length; i++) {
		if (nodes[i].containsPoint(x, y)) {
			return nodes[i];
		}
	}
	for (var i = 0; i < links.length; i++) {
		if (links[i].containsPoint(x, y)) {
			return links[i];
		}
	}
	return null;
}

function snapNode(node) {
	for (var i = 0; i < nodes.length; i++) {
		if (nodes[i] == node) continue;
			  
			  if (Math.abs(node.x - nodes[i].x) < snapToPadding) {
				  node.x = nodes[i].x;
			  }
			  
			  if (Math.abs(node.y - nodes[i].y) < snapToPadding) {
				  node.y = nodes[i].y;
			  }
	}
}

function resizecanvas() {
	if (canvas == null)
		return;
	canvas.width=$("#righteditpanel").width();
	canvas.height = $("#righteditpanel").height()-$("#sceneheader").height()-$("#scenefooter").height()-10;
	console.log("resize ",canvas.width);
	draw();
}

$(window).resize(function() {resizecanvas();});

function canvasHasFocus() {
	return document.activeElement == canvas;
}

$(document).one('pageinit', function() {
	canvas = document.getElementById('canvas');
	$canvas = $(canvas);
	//restoreBackup();
	setTimeout( "resizecanvas();",500 );
	
	$canvas.mousedown(function(e) {
		// set focus to canvas (to get key strokes)
		$canvas.attr("tabindex", "0");
		// mouse position
		var mouse = {'x': e.pageX - $canvas.offset().left,'y': e.pageY - $canvas.offset().top};
		if (originalMouse == null)
			originalMouse = mouse;
		selectedObject = selectObject(mouse.x, mouse.y);
		movingObject = false;
		
		if (selectedObject != null) {
			if (shift && selectedObject instanceof Node) {
				$canvas.css('cursor', 'pointer');
				currentLink = new TemporaryLink(selectedObject, mouse);
			} else {
				$canvas.css('cursor', 'move');
				movingObject = true;
				if (selectedObject.setMouseStart) {
					selectedObject.setMouseStart(mouse.x, mouse.y);
				}
			}
		} else {
			$canvas.css('cursor', 'move');
			// Determine max scroll area
// 			scrollMaxX = 0; scrollMaxY = 0;
// 			for (var i = 0; i < nodes.length; i++) {
// 				if (nodes[i].x+nodes[i].width>scrollMaxX)
// 					scrollMaxX = nodes[i].x+nodes[i].width;
// 				if (nodes[i].y+nodes[i].height>scrollMaxY)
// 					scrollMaxY = nodes[i].y+nodes[i].height;
// 			}
		}
		draw();
		
		if (canvasHasFocus()) {
			// disable drag-and-drop only if the canvas is already focused
			return false;
		} else {
			// otherwise, let the browser switch the focus away from wherever it was
			return true;
		}
	});
	
	canvas.ondblclick = function(e) {
		var mouse = {'x': e.pageX - $canvas.offset().left,'y': e.pageY - $canvas.offset().top};
		selectedObject = selectObject(mouse.x, mouse.y);
		
		if (selectedObject != null && selectedObject instanceof Node) {
// 			selectedObject = new Node(mouse.x, mouse.y);
// 			selectedObject.setText(nodeText());
// 			nodes.push(selectedObject);
// 			draw();
			showSceneItemDialog(selectedObject.data, false);
		}
	};
	
	$canvas.mousemove(function(e) {
		if (currentLink != null) {
			var mouse = {'x': e.pageX - $canvas.offset().left,'y': e.pageY - $canvas.offset().top}; 
			var targetNode = selectObject(mouse.x, mouse.y);
			if (!(targetNode instanceof Node)) {
				targetNode = null;
			}
			
			if (selectedObject != null) {
				if (targetNode != null) {
					currentLink = new Link(selectedObject, targetNode);
				} else {
					currentLink = new TemporaryLink(selectedObject.closestPointOnShape(mouse.x, mouse.y), mouse);
				}
			}
			draw();
		} else if (movingObject) {
			var mouse = {'x': e.pageX - $canvas.offset().left,'y': e.pageY - $canvas.offset().top}; 
			selectedObject.setAnchorPoint(mouse.x, mouse.y);
			if (selectedObject instanceof Node) {
				snapNode(selectedObject);
			}
			draw();
		} else if (originalMouse != null) { // move nodes
			var mouse = {'x': e.pageX - $canvas.offset().left,'y': e.pageY - $canvas.offset().top}; 
			for (var i = 0; i < nodes.length; i++) {
// 				if (scrollMaxX<mouse.x)
					nodes[i].x += mouse.x - originalMouse.x;
// 				if (scrollMaxY<mouse.y)
					nodes[i].y += mouse.y - originalMouse.y;
			}
			draw();
			originalMouse = mouse;
		}
	});
	
	$canvas.mouseup(function(e) {
		movingObject = false;
		originalMouse = null;
		if (currentLink != null) {
			if (!(currentLink instanceof TemporaryLink)) {
				selectedObject = currentLink;
				links.push(currentLink);
			}
			currentLink = null;
			draw();
		}
		$canvas.css('cursor', 'default');
	});
	
	
	$canvas.keydown(function(event) {
		var key = event.which;
		
		if (key == 16) {
			shift = true;
		} else if (selectedObject != null) {
			if (key == 8 || key == 46) { // delete key
				for (var i = 0; i < nodes.length; i++) {
					if (nodes[i] == selectedObject) {
						nodes.splice(i--, 1);
					}
				}
				for (var i = 0; i < links.length; i++) {
					if (links[i] == selectedObject || links[i].node == selectedObject || links[i].nodeA == selectedObject || links[i].nodeB == selectedObject) {
						links.splice(i--, 1);
					}
				}
				selectedObject = null;
				draw();
			}
		}
		
		// backspace is a shortcut for the back button, but do NOT want to change pages
		if (key == 8) return false;
	});
	
	$canvas.keyup(function(event) {
		var key = event.which;
		
		if (key == 16) {
			shift = false;
		}
	});
});

function nodeText(nodeIndex) {
	if (nodeIndex==null)
		nodeIndex = nodes.length;
	return "Node "+nodeIndex;
}

function indexOfSceneItem(sceneItem) {
	for (var i = 0; i < nodes.length; i++) {
		var node = nodes[i].data;
		if (node.type_==sceneItem.type_ && node.id_==sceneItem.id_)
			return i;
	}
	return -1;
}

function loadSceneItemsFromScene(scene) {
	nodes = [];
	links = [];
	
	//var sceneDocuments = storageInstance.documentsForScene(sceneid);
// 	console.log("load scene", scene);
	var padding = 50;
	var startx = 0; var starty = 0;

	var c = canvas.getContext('2d');
	
	for (var i = 0; i < scene.v.length; i++) {
		var sceneNode = scene.v[i];
		var sceneItem = storageInstance.getDocument(sceneNode.type_, sceneNode.id_);
		if (sceneItem==null) {
			console.warn("Cannot find scene item "+sceneNode.type_+ " " + sceneNode.id_);
			continue;
		}
		var node = new Node();
		node.setData(c, sceneItem);
		nodes.push(node);
		
		if (sceneNode.draw_ == null) {
			if (i==0) {
				startx += node.width;
				starty += node.height;
			}
			if (startx>canvas.width) {
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
			var nodeIndex = indexOfSceneItem(sceneNode.e[l]);
			if (nodeIndex==-1) {
				console.warn("Node not found", sceneNode.e[l]);
				continue;
			}
			var link = new Link(nodes[i], nodes[nodeIndex]);
			links.push(link);
		}
	}
	draw();
}
/*
function restoreBackup() {
	if (!localStorage || !JSON) {
		return;
	}
	
	try {
		var backup = JSON.parse(localStorage['fsm']);
		
		for (var i = 0; i < backup.v.length; i++) {
			var backupNode = backup.v[i];
			var node = new Node(backupNode.draw_.x, backupNode.draw_.y);
			node.setText(nodeText(i));
			nodes.push(node);
		}
		
		for (var i = 0; i < backup.v.length; i++) {
			var backupNode = backup.v[i];
			for (var l = 0; l < backupNode.e.length; l++) {
				var backupLink = backupNode.e[l];
				var link = new Link(nodes[i], nodes[backupLink.nodeB]);
				link.parallelPart = backupLink.draw_.parallelPart;
				link.perpendicularPart = backupLink.draw_.perpendicularPart;
				links.push(link);
			}
		}
	} catch(e) {
		localStorage['fsm'] = '';
	}
}

function saveBackup() {
	if (!localStorage || !JSON) {
		return;
	}
	
	var backup = { 'v': [] };
	for (var i = 0; i < nodes.length; i++) {
		var node = nodes[i];
		
		var backupNode = {'draw_':{'x': node.x,'y': node.y}, 'e':[]};
		for (var l = 0; l < links.length; l++) {
			var link = links[l];
			if (link.nodeA!=nodes[i])
				continue;
			var backupLink = {
				'nodeB': nodes.indexOf(link.nodeB),
			'draw_':{
				'parallelPart': link.parallelPart,
				'perpendicularPart': link.perpendicularPart
			}
			};
			backupNode.e.push(backupLink);
		}
		backup.v.push(backupNode);
	}
	localStorage['fsm'] = JSON.stringify(backup);
}
	*/