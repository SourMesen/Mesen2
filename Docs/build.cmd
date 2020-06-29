hugo
del docs\js\searchjson.js
echo var searchjson = >>docs\js\searchjson.js
type docs\index.json>>docs\js\searchjson.js
