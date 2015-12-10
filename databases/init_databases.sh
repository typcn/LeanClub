#!/bin/bash

JSON_LIST=`ls *.json`

for i in $JSON_LIST
do

	i=${i%.json}
	echo $i
	curl -d @$i.json -H 'Content-type: application/json' -X POST http://$1/$i/_bulk_docs

done
