$(function(){
	$(".tooltip").tipTip();
	$(".content h1").append('<a style="float:right;font-size:0.5em" href="#"><i class="icon-arrow-up"></i>Nach oben</a>');

	window.updatelastIssue = function(response) {
		var meta = response.meta
		var data = response.data
		console.log(meta)
		console.log(data)
	}
	
	if(typeof(Storage)!=="undefined")
	{
		window.refreshIssuesData = function(callback) {
			if (callback == null) return;
			var oldDate = new Date(localStorage.lastIssueAccess);
			oldDate.setSeconds(oldDate.getSeconds() + 60*5);
  			if (oldDate > new Date()) {
				console.log("cache");
				// use cache value
				var data = JSON.parse(localStorage.lastOpenIssues);
				callback(data);
			} else {
				$.getJSON("https://api.github.com/repos/davidgraeff/scenecontrol/issues?callback=?",
					{state: "open"},
					function(response) {
						// update cache value 
						localStorage.lastIssueAccess = new Date();
						localStorage.lastOpenIssues = JSON.stringify(response.data);
						var data = response.data;
						callback(data);
					}
				);
			}
		}
	}
	else
	{
		window.refreshIssuesData = function() {}
	}
});
