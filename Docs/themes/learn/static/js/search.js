var lunrIndex, pagesIndex;

function endsWith(str, suffix) {
    return str.indexOf(suffix, str.length - suffix.length) !== -1;
}

// Initialize lunrjs using our generated index file
function initLunr() {
    // First retrieve the index file
	pagesIndex =   searchjson;
	// Set up lunrjs by declaring the fields we use
	// Also provide their boost level for the ranking
	lunrIndex = new lunr.Index
	lunrIndex.ref("uri");
	lunrIndex.field('title', {
		boost: 15
	});
	lunrIndex.field('tags', {
		boost: 10
	});
	lunrIndex.field("content", {
		boost: 5
	});

	// Feed lunr with each file and let lunr actually index them
	pagesIndex.forEach(function(page) {
		lunrIndex.add(page);
	});
	lunrIndex.pipeline.remove(lunrIndex.stemmer)
}

/**
 * Trigger a search in lunr and transform the result
 *
 * @param  {String} query
 * @return {Array}  results
 */
function search(query) {
    // Find the item in our index corresponding to the lunr one to have more info
    return lunrIndex.search(query).map(function(result) {
            return pagesIndex.filter(function(page) {
                return page.uri === result.ref;
            })[0];
        });
}

// Let's get started
initLunr();

var scriptPath = document.currentScript.getAttribute("src");
var depth = 0;
while(scriptPath.indexOf("..") >= 0) {
	depth++;
	scriptPath = scriptPath.substr(scriptPath.indexOf("..") + 2);
}

$( document ).ready(function() {
    var horseyList = horsey($("#search-by").get(0), {
        suggestions: function (value, done) {
            var query = $("#search-by").val();
            var results = search(query);
            done(results);
        },
        filter: function (q, suggestion) {
            return true;
        },
        set: function (value) {
			var prefix = "";
			for(var i = 0; i < depth; i++) {
				prefix += "../";
			}
			if(!prefix) {
				prefix = "./"
			}
            location.href=prefix+value.uri.substr(1);
        },
        render: function (li, suggestion) {
            var uri = suggestion.uri.substring(1,suggestion.uri.length);

			
			
            suggestion.href = uri;

            var query = $("#search-by").val();
            var numWords = 2;
			var regex = new RegExp("(?:\\s?(?:[\\w]+)\\s?){0,"+numWords+"}"+query+"(?:\\s?(?:[\\w]+)\\s?){0,"+numWords+"}", "i");
            var text = suggestion.content.match(regex);
            suggestion.context = text;
            var image = '<div>' + '» ' + suggestion.title + '</div><div style="font-size:12px">' + (suggestion.context || '') +'</div>';
            li.innerHTML = image;
        },
        limit: 10
    });
    horseyList.refreshPosition();
});
