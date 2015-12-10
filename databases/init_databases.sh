#!/bin/bash


# example:
# chmod u+x init_databases.sh
# ./init_databases.sh 127.0.0.1:5984
#

JSON_LIST=`ls *.json`

for i in $JSON_LIST
do

	i=${i%.json}
	echo $i
	curl -d @$i.json -H 'Content-type: application/json' -X POST http://$1/$i/_bulk_docs

done
