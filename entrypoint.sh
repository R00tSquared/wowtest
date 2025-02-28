#!/bin/bash

echo "Starting HellGround Server..."

# Start MySQL if not running
service mysql start

# Ensure the database is set up (if needed)
mysql -u root -e "CREATE DATABASE IF NOT EXISTS hellground_db;"

# Run the realm and world server
/mwcore/bin/hellgroundcore &
/mwcore/bin/hellgroundrealm &
wait
