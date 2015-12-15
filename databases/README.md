curl -d @database_name.json -H "Content-type: application/json" -X POST http://DATABASE_URL/[database_name]/_bulk_docs

You can also use init_databases.sh