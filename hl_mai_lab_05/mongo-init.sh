set -e

mongo <<EOF
db = db.getSiblingDB('mongo_db')
db.createCollection('message')
db.createCollection('post')
db.message.insertOne({"id": 1, "sender_id": 1, "recipient_id": 3, "text": "Hello!", "date": "16.05.2024"})
db.post.insertOne({"id": 1, "user_id": 1, "text": "Hello World!", "date": "10.05.2024"})
EOF