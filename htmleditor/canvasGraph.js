/**
 * Canvas Module - For drawing nodes and lines on a canvas
 * Available as Link, Node, TemporaryLink.
 */
(function (window) {
	"use strict";
	var canvasConfig = {
		nodeRadius: 30,
		snapToPadding: 6,
		hitTargetPadding: 6
	};

	/////////////// Canvas extensions //////////////////
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

	CanvasRenderingContext2D.prototype.drawArrow = function(x, y, angle) {
		var dx = Math.cos(angle);
		var dy = Math.sin(angle);
		this.beginPath();
		this.moveTo(x, y);
		this.lineTo(x - 8 * dx + 5 * dy, y - 8 * dy - 5 * dx);
		this.lineTo(x - 8 * dx - 5 * dy, y - 8 * dy + 5 * dx);
		this.fill();
	}

	CanvasRenderingContext2D.prototype.fragmentText = function (text, maxWidth) {
		var words = text.split(' ');
		var lines = [];
		var line = "";
		
			if (this.measureText(text).width < maxWidth) {
				return [text];
			}
			while (words.length > 0) {
				while (this.measureText(words[0]).width >= maxWidth) {
					var tmp = words[0];
					words[0] = tmp.slice(0, -1);
					if (words.length > 1) {
						words[1] = tmp.slice(-1) + words[1];
					} else {
						words.push(tmp.slice(-1));
					}
				}
				if (this.measureText(line + words[0]).width < maxWidth) {
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

	////////// Nodes and Links ////////////
	window.Link = function (a, b) {
		this.nodeA = a;
		this.nodeB = b;
		this.enabled = true;
		
		// make anchor point relative to the locations of nodeA and nodeB
		this.parallelPart = 0.5; // percentage from nodeA to nodeB
		this.perpendicularPart = 0; // pixels from line between nodeA and nodeB
	}
	
	Link.prototype.setEnable = function(b) {
		this.enabled = b;
	};
	
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
		var circle = MathHelper.circleFromThreePoints(this.nodeA.x, this.nodeA.y, this.nodeB.x, this.nodeB.y, anchor.x, anchor.y);
		var isReversed = (this.perpendicularPart > 0);
		var reverseScale = isReversed ? 1 : -1;
		var startAngle = Math.atan2(this.nodeA.y - circle.y, this.nodeA.x - circle.x) - reverseScale * canvasConfig.nodeRadius / circle.radius;
		var endAngle = Math.atan2(this.nodeB.y - circle.y, this.nodeB.x - circle.x) + reverseScale * canvasConfig.nodeRadius / circle.radius;
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
			c.drawArrow(stuff.endX, stuff.endY, stuff.endAngle - stuff.reverseScale * (Math.PI / 2));
		} else {
			c.drawArrow(stuff.endX, stuff.endY, Math.atan2(stuff.endY - stuff.startY, stuff.endX - stuff.startX));
		}
	};

	Link.prototype.containsPoint = function(x, y) {
		var stuff = this.getEndPointsAndCircle();
		if (stuff.hasCircle) {
			var dx = x - stuff.circleX;
			var dy = y - stuff.circleY;
			var distance = Math.sqrt(dx*dx + dy*dy) - stuff.circleRadius;
			if (Math.abs(distance) < canvasConfig.hitTargetPadding) {
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
			return (percent > 0 && percent < 1 && Math.abs(distance) < canvasConfig.hitTargetPadding);
		}
		return false;
	};

	window.Node = function () {
		this.mouseOffsetX = 0;
		this.mouseOffsetY = 0;
		this.fontsize = 20;
		this.width = 0;
		this.maxwidth = 200;
		this.height = 0;
		this.textPadding = 10;
		this.headerHeight = 25 + 2 * this.textPadding;
		this.cachetext = [];
		this.enabled = true;
	}

	Node.prototype.setEnable = function(b) {
		this.enabled = b;
	};
	
	Node.prototype.snap = function(otherNode) {
		if (this == otherNode) return;
		
		if (Math.abs(otherNode.x - this.x) < canvasConfig.snapToPadding) {
			node.x = this.nodes[i].x;
		}
		
		if (Math.abs(otherNode.y - this.y) < canvasConfig.snapToPadding) {
			otherNode.y = this.y;
		}
	};
	
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
				text = SceneItem.text( this.data, this.dataschema );
			} else {
				text = this.dataschema.name;
			}
		} else {
			// No schema: We do not know how to express this item in text
			text = JSON.stringify(this.data);
		}
		
		// determine lines
		this.cachetext = c.fragmentText(text, this.maxwidth);
		
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
		c.font = this.fontsize+'px "Times New Roman", serif';
		var w2 = this.width/2;
		var h2 = this.height/2;
		
		// draw main rectangle

		c.lineWidth = 2;
		c.fillStyle = 'white';
		c.save();
		c.shadowColor = '#999';
		c.shadowBlur = 10;
		c.shadowOffsetX = 3;
		c.shadowOffsetY = 3;
		c.roundRect(this.x-w2, this.y-h2, this.width, this.height, 10);
		c.fill();
		c.restore();
		c.stroke();
		
		// clip to the outer rectangle
		
		{ // gradient
		c.save();
		c.clip();
		var lingrad = c.createLinearGradient(0, this.y-h2, 0, this.y+h2);
		lingrad.addColorStop(0.0, 'white');
		lingrad.addColorStop(1.0, 'blue');
		c.fillStyle = lingrad;
		c.fillRect(this.x-w2, this.y-h2, this.width, this.headerHeight);
		// 		c.stroke();
		c.restore();
		}
		
		// draw outer line again
		c.stroke();
		
		c.shadowColor = '#999';
		c.shadowBlur = 5;
		c.shadowOffsetX = 2;
		c.shadowOffsetY = 2;
		
		{ // Draw title
		c.fillStyle = 'black';
		c.fillText(this.data.type_, this.x-w2+this.textPadding, this.y-h2+this.headerHeight/2);
		c.save();
		c.font = (this.fontsize-5)+'px "Times New Roman", serif';
		c.fillText(this.data.componentid_, this.x-w2+this.textPadding, this.y-h2+this.headerHeight/2+12);
		c.restore();
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

	window.TemporaryLink = function (from, to) {
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
		c.drawArrow(this.to.x, this.to.y, Math.atan2(this.to.y - this.from.y, this.to.x - this.from.x));
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	/////// Helper methods
	function MathHelper() {
		function det(a, b, c, d, e, f, g, h, i) {
			return a*e*i + b*f*g + c*d*h - a*f*h - b*d*i - c*e*g;
		}

		function circleFromThreePoints(x1, y1, x2, y2, x3, y3) {
			var a = MathHelper.det(x1, y1, 1, x2, y2, 1, x3, y3, 1);
			var bx = -MathHelper.det(x1*x1 + y1*y1, y1, 1, x2*x2 + y2*y2, y2, 1, x3*x3 + y3*y3, y3, 1);
			var by = MathHelper.det(x1*x1 + y1*y1, x1, 1, x2*x2 + y2*y2, x2, 1, x3*x3 + y3*y3, x3, 1);
			var c = -MathHelper.det(x1*x1 + y1*y1, x1, y1, x2*x2 + y2*y2, x2, y2, x3*x3 + y3*y3, x3, y3);
			return {
				'x': -bx / (2*a),
				'y': -by / (2*a),
				'radius': Math.sqrt(bx*bx + by*by - 4*a*c) / (2*Math.abs(a))
			};
		}
	};
})(window);
