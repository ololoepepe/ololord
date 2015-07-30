#!/bin/bash
SOURCE=$1

if [ -z "$SOURCE" ]; then
    echo "No input file specified"
    exit 1
fi

TMP="${SOURCE}.tmp"
DIR=$( cd "$( dirname "$TMP" )" && pwd )

echo "Creating a temporary copy of source file..."
cp "$SOURCE" "$TMP"
echo "OK"

echo "Replacing seperators..."
sed -i 's#"\,"#\|#g' "$TMP"
echo "OK"

echo "Removing double quotes..."
sed -i 's#"##g' "$TMP"
echo "OK"

echo "Removing empty lines..."
sed -i '/^\s*$/d' "$TMP"
echo "OK"

echo "Removing first line..."
echo "$(tail -n +2 "$TMP")" > "$TMP"
echo "OK"

echo "Importing data into a database..."
sqlite3 "$DIR/ip2location.sqlite" <<EndOfScript

CREATE TABLE ip2location (
    ip_from INTEGER,
    ip_to INTEGER,
    country_code TEXT,
    country_name TEXT,
    region_name TEXT,
    city_name TEXT
);
CREATE INDEX idx_ip_from ON ip2location (
    ip_from
);
CREATE INDEX idx_ip_to ON ip2location (
    ip_to
);
CREATE INDEX idx_ip_from_ip_to ON ip2location (
    ip_from,
    ip_to
);
.separator "|"
.import "$TMP" ip2location

EndOfScript
echo "OK"

echo "Removing temporary file..."
rm "$TMP"
echo "OK"

echo "Done!"
