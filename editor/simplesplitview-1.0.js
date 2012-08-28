/*
 * jQuery plugin SimpleSplitView v1.0.1
 * http://newbound.com/simplesplitview
 * March 21, 2012
 * 
 * Copyright 2012, Peter Yared & Marc Raiser
 * Dual licensed under the MIT or GPL Version 2 licenses.
 * http://jquery.org/license
 */

(function( $ ){
	function internalInit(container) {
		var myid = container.id;
		var newhtml = 
			"<table border='0' cellpadding='0' cellspacing='0' width='100%' height='100%'>\
				<tr>\
					<td valign='top' width='400px' id='"+myid+"_leftpagepanel' style='position:relative; overflow: hidden;'>\
						<div id='"+myid+"_leftcontainer' style='height: 100%; width: 100%;'>\
						</div>\
					</td>\
					<td id='"+myid+"_rightpagepanel' valign='top' style='position:relative; overflow: hidden; border-left-style: solid; border-left-width: thin; border-left-color: black;'>\
						<div id='"+myid+"_rightcontainer' style='height: 100%; width: 100%;'>\
						</div>\
					</td>\
				</tr>\
			</table>";
		
		var kids = container.childNodes;
		var leftkids = [];
		var rightkids = [];
		var re1 = new RegExp('\\bleftside\\b');
		var re2 = new RegExp('\\brightside\\b');
		for (var node in kids) {
			if (re1.test(kids[node].className)) leftkids.push(kids[node]);
			else if (re2.test(kids[node].className)) rightkids.push(kids[node]);
		}
		
		container.style.position='fixed';
		container.style.overflow='hidden';
		container.style.width='100%';
		container.style.height='100%';
		container.style.padding='0px';
		container.innerHTML = newhtml;
		
		container.leftcontainer = $('#'+myid+'_leftcontainer');
		container.rightcontainer = $('#'+myid+'_rightcontainer');
		container.usesplitview = true;
		container.leftpagestack = null;
		container.rightpagestack = null;
		container.showRight = methods.showRight;
		
		$(window).bind('resize.simplesplitview', checkResolution);
		checkResolution(true);

		var pad = 0;
		if (container.padding) pad = container.padding;

		for (var node in leftkids) {
			var k = leftkids[node];
			k.style.position = 'relative';
			k.style.overflow = 'auto';
			if (node == 0) {
				k.style.display = 'block';
				k.style.height = ($('#'+container.id+'_leftpagepanel').height()-pad)+"px";
				var o = new Object();
				o.name = k.id;
				container.leftpagestack = o;
			}
			else k.style.display = 'none';
			container.leftcontainer.append(k);
		}				

		for (var node in rightkids) {
			var k = rightkids[node];
			k.style.position = 'relative';
			k.style.overflow = 'auto';
			if (node == 0){
				k.style.display = 'block';
				k.style.height = ($('#'+container.id+'_rightpagepanel').height()-pad)+"px";
				var o = new Object();
				o.name = k.id;
				container.rightpagestack = o;
				if (container.usesplitview && container.leftpagestack != null) container.leftpagestack.right = o.name;
			}
			else k.style.display = 'none';
			container.rightcontainer.append(k);
		}				
	}
	
	function checkResolution(sync){
		$('.splitviewcontainer').each(function(){
			var me = this;
			function doResize(){
				var w = me.parentNode.offsetWidth;
				me.usesplitview = w > 700;
				if (me.usesplitview) {
					$('#'+me.id+'_leftcontainer').css('display', 'block');
					$('#'+me.id+'_rightcontainer').css('display', 'block');
					$('#'+me.id+'_leftpagepanel').width(400);
					$('#'+me.id+'_rightpagepanel').width(w-400);
				}
				else {
					$('#'+me.id+'_rightcontainer').css('display', 'none');
					$('#'+me.id+'_rightpagepanel').width(0);
					$('#'+me.id+'_leftpagepanel').width(w);
				}
				var h = me.parentNode.offsetHeight;
				var pad = 0;
				if (me.padding) pad = me.padding;
				$('.leftside').height(h-pad);
				$('.rightside').height(h-pad);
			}
			doResize();
			setTimeout(doResize, 1000);
		});
	}
	
	function slide(divid, pixels, millis){
		var el = document.getElementById(divid);
		el.startTime = new Date().getTime();
		el.pixels = pixels;
		el.millis = millis;
		el.slide = function(){
			var percent = (new Date().getTime() - el.startTime) / millis;
			var left = el.pixels - (percent * el.pixels);
			if (left <= 0) left = 0;
			el.style.left = left+'px';
			if (left > 0) setTimeout(el.slide, 10);
		};
		
		el.slide();
	}
	
	function showLeftInternal(container, leftpage, rightpage, dontslideright, dontslideleft){
		if (!container.usesplitview){
			var w = container.parentNode.offsetWidth;
			$('#'+container.id+'_leftcontainer').css('display', 'block');
			$('#'+container.id+'_rightcontainer').css('display', 'none');
			$('#'+container.id+'_leftpagepanel').width(w);
			$('#'+container.id+'_rightpagepanel').width(0);
		}

		if (container.leftpagestack != null) {
			$('#'+container.leftpagestack.name).css('display', 'none');
			$('#'+container.id+'_homebutton').css('display', 'block');
			$('#'+container.id+'_backbutton').css('display', 'block');
		}
		
		var o = new Object();
		o.name = leftpage;
		o.prev = container.leftpagestack;
		o.right = rightpage;
		
		if (container.leftpagestack == null || container.leftpagestack.name != o.name) container.leftpagestack = o;

		if (dontslideleft) {}
		else slide(leftpage, $('#'+container.id+'_leftpagepanel').width(), 500);

		var pad = 0;
		if (container.padding) pad = container.padding;
		$('#'+leftpage).height($('#'+container.id+'_leftpagepanel').height()-pad);
		$('#'+leftpage).css('display', 'block');
		
		if (container.usesplitview && rightpage) showRightInternal(container, rightpage, dontslideright);
	}
	
	function showRightInternal(container, rightdiv, dontslide){
		if (!container.usesplitview){
			var w = container.parentNode.offsetWidth;
			$('#'+container.id+'_leftcontainer').css('display', 'none');
			$('#'+container.id+'_rightcontainer').css('display', 'block');
			$('#'+container.id+'_leftpagepanel').width(0);
			$('#'+container.id+'_rightpagepanel').width(w);
			$('#'+container.id+'_homebutton').css('display', 'block');
			$('#'+container.id+'_backbutton').css('display', 'block');
		}

		if (container.rightpagestack != null) {
			$('#'+container.rightpagestack.name).css('display', 'none');
		}
		
		var o = new Object();
		o.name = rightdiv;
		o.prev = container.rightpagestack;
		container.rightpagestack = o;
		
		if (dontslide) {}
		else slide(rightdiv, $('#'+container.id+'_rightpagepanel').width(), 500);
		
		var pad = 0;
		if (container.padding) pad = container.padding;
		$('#'+rightdiv).height($('#'+container.id+'_rightpagepanel').height()-pad);
		$('#'+rightdiv).css('display', 'block');
	}
	
	function navHomeInternal(container){
		if (container.leftpagestack != null) $('#'+container.leftpagestack.name).css('display', 'none');
		if (container.rightpagestack != null) $('#'+container.rightpagestack.name).css('display', 'none');
		
		$('#'+container.id+'_homebutton').css('display', 'none');
		$('#'+container.id+'_backbutton').css('display', 'none');

		var gl;
		var gr;
		while (container.leftpagestack != null) {
			gl = container.leftpagestack.name;
			gr = container.leftpagestack.right;
			container.leftpagestack = container.leftpagestack.prev;
		}
		
		container.rightpagestack = null;
			
		showLeftInternal(container, gl, gr);
	}
	
	function navBackInternal(container){
		var isleft = $('#'+container.id+'_leftpagepanel').width() == 0;
		
		if (!container.usesplitview) {
			var w = container.parentNode.offsetWidth;
			
			if (!isleft) {
				container.leftpagestack = container.leftpagestack.prev;
				if (container.leftpagestack.right) {
					var o = new Object();
					o.name = container.leftpagestack.right;
					o.prev = container.rightpagestack;
					container.rightpagestack = o;
				}
				else isleft = true;
			}
			
			if (isleft) {
				$('.leftside').css('display', 'none');
				$('#'+container.leftpagestack.name).css('display', 'block');
				
				$('#'+container.id+'_leftcontainer').css('display', 'block');
				$('#'+container.id+'_rightcontainer').css('display', 'none');
				$('#'+container.id+'_rightpagepanel').width(0);
				$('#'+container.id+'_leftpagepanel').width(w);
				
				var display = container.leftpagestack.prev == null ? 'none' : 'block';
				$('#'+container.id+'_homebutton').css('display', display);
				$('#'+container.id+'_backbutton').css('display', display);
				
				slide(container.leftpagestack.name, $('#'+container.id+'_leftpagepanel').width(), 500);
			}
			else {
				$('.rightside').css('display', 'none');
				$('#'+container.rightpagestack.name).css('display', 'block');

				$('#'+container.id+'_leftcontainer').css('display', 'none');
				$('#'+container.id+'_rightcontainer').css('display', 'block');
				$('#'+container.id+'_leftpagepanel').width(0);
				$('#'+container.id+'_rightpagepanel').width(w);

				$('#'+container.id+'_homebutton').css('display', 'block');
				$('#'+container.id+'_backbutton').css('display', 'block');

				slide(container.rightpagestack.name, $('#'+container.id+'_rightpagepanel').width(), 500);
			}
		}
		else {
			$('#'+container.leftpagestack.name).css('display', 'none');
			$('#'+container.rightpagestack.name).css('display', 'none');
			
			$('#'+container.id+'_homebutton').css('display', 'none');
			$('#'+container.id+'_backbutton').css('display', 'none');
			
			var o = container.leftpagestack.prev;
			container.leftpagestack = o.prev;
			var dontslideright = container.rightpagestack.name == o.right;
			showLeftInternal(container, o.name, o.right, dontslideright);
		}
	}
	
	var methods = {
		init : function( options ) {
			return this.each(function(){
				if (document.getElementById(this.id+'_leftpagepanel') == null) internalInit(this);
			});
		},
		destroy : function( ) {
			return this.each(function(){
				$(window).unbind('.simplesplitview');this.id
			})
		},
		showLeft : function(leftpage, rightpage, dontslideright, dontslideleft) { 
			return this.each(function(){
				showLeftInternal(this, leftpage, rightpage, dontslideright, dontslideleft);
			});
		},
		showRight : function( rightdiv, dontslide ) {
			return this.each(function(){
				showRightInternal(this, rightdiv, dontslide);
			});
		},
		navHome : function() {
			return this.each(function(){
				navHomeInternal(this);
			});
		},
		navBack : function() {
			return this.each(function(){
				navBackInternal(this);
			});
		}
	};

	$.fn.simplesplitview = function( method ) {
		if ( methods[method] ) {
			return methods[method].apply( this, Array.prototype.slice.call( arguments, 1 ));
		} else if ( typeof method === 'object' || ! method ) {
			return methods.init.apply( this, arguments );
		} else {
			$.error( 'Method ' +  method + ' does not exist on jQuery.simplesplitview' );
		}    
	};
})( jQuery );

