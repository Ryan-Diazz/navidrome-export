#!/bin/bash
# Navidrome Subsonic API Diagnostic
# Tests actual API responses to debug issues

set -e

echo "╔════════════════════════════════════════════════════════════════════╗"
echo "║      Navidrome Subsonic API Diagnostic - Debug Song Fetching      ║"
echo "╚════════════════════════════════════════════════════════════════════╝"
echo ""

# Get config
CONFIG_FILE="$HOME/.config/navidrome-export/config.json"

if [ ! -f "$CONFIG_FILE" ]; then
    echo "❌ Error: Config file not found at $CONFIG_FILE"
    echo "   Run: navidrome-export init"
    exit 1
fi

# Extract values from JSON
URL=$(grep '"navidrome_url"' "$CONFIG_FILE" | sed 's/.*: "\(.*\)".*/\1/')
USER=$(grep '"username"' "$CONFIG_FILE" | sed 's/.*: "\(.*\)".*/\1/')
PASS=$(grep '"password"' "$CONFIG_FILE" | sed 's/.*: "\(.*\)".*/\1/')

# Remove trailing slash
URL="${URL%/}"

echo "Configuration:"
echo "  URL: $URL"
echo "  User: $USER"
echo ""

ENC_PASS="enc:${PASS}"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "TEST 1: Ping API"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

PING_RESPONSE=$(curl -s "${URL}/rest/ping.view?u=${USER}&p=${ENC_PASS}&c=test&v=1.12.0&f=json")
echo "Response:"
echo "$PING_RESPONSE" | jq . 2>/dev/null || echo "$PING_RESPONSE"
echo ""

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "TEST 2: Get Album List (first 10)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

ALBUMS_RESPONSE=$(curl -s "${URL}/rest/getAlbumList2.view?u=${USER}&p=${ENC_PASS}&c=test&v=1.12.0&f=json&type=alphabeticalByName&size=10")
echo "Response:"
echo "$ALBUMS_RESPONSE" | jq . 2>/dev/null || echo "$ALBUMS_RESPONSE"
echo ""

# Extract album count
ALBUM_COUNT=$(echo "$ALBUMS_RESPONSE" | jq '.["subsonic-response"].albumList2 | length' 2>/dev/null || echo "0")
TOTAL_ALBUMS=$(echo "$ALBUMS_RESPONSE" | jq '.["subsonic-response"].total' 2>/dev/null || echo "0")

echo "Summary:"
echo "  Albums in response: $ALBUM_COUNT"
echo "  Total albums on server: $TOTAL_ALBUMS"
echo ""

if [ "$ALBUM_COUNT" -eq 0 ]; then
    echo "⚠️  WARNING: No albums returned!"
    echo "   Possible causes:"
    echo "   1. No music imported in Navidrome"
    echo "   2. Different API structure (check Navidrome version)"
    echo "   3. API access issue"
    echo ""
fi

# Get first album ID
FIRST_ALBUM_ID=$(echo "$ALBUMS_RESPONSE" | jq -r '.["subsonic-response"].albumList2[0].id' 2>/dev/null)

if [ -z "$FIRST_ALBUM_ID" ] || [ "$FIRST_ALBUM_ID" == "null" ]; then
    echo "❌ Could not extract first album ID"
    echo "   Full response structure:"
    echo "$ALBUMS_RESPONSE" | jq . 2>/dev/null || echo "$ALBUMS_RESPONSE"
    exit 1
fi

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "TEST 3: Get Songs from First Album (ID: $FIRST_ALBUM_ID)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

ALBUM_RESPONSE=$(curl -s "${URL}/rest/getAlbum.view?u=${USER}&p=${ENC_PASS}&c=test&v=1.12.0&f=json&id=${FIRST_ALBUM_ID}")
echo "Response:"
echo "$ALBUM_RESPONSE" | jq . 2>/dev/null || echo "$ALBUM_RESPONSE"
echo ""

# Extract song count
SONG_COUNT=$(echo "$ALBUM_RESPONSE" | jq '.["subsonic-response"].album.song | length' 2>/dev/null || echo "0")

echo "Summary:"
echo "  Songs in this album: $SONG_COUNT"
echo ""

if [ "$SONG_COUNT" -eq 0 ]; then
    echo "⚠️  WARNING: No songs in first album!"
    echo "   Possible causes:"
    echo "   1. Albums exist but have no songs (metadata only?)"
    echo "   2. Different API structure"
    echo "   3. Songs stored differently"
    echo ""
fi

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "DIAGNOSIS"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

if [ "$ALBUM_COUNT" -eq 0 ]; then
    echo "❌ ISSUE: No albums found"
    echo ""
    echo "This means either:"
    echo "  1. No music is imported in Navidrome"
    echo "  2. The API endpoint is returning an unexpected format"
    echo "  3. Your Navidrome version uses a different API"
    echo ""
elif [ "$SONG_COUNT" -eq 0 ]; then
    echo "❌ ISSUE: Albums found but no songs"
    echo ""
    echo "This means:"
    echo "  1. $ALBUM_COUNT albums exist"
    echo "  2. But they have no songs (or different structure)"
    echo ""
    echo "Check:"
    echo "  - Full response structure above"
    echo "  - Maybe songs are stored under different key?"
    echo "  - Maybe albums need different API call?"
    echo ""
else
    echo "✅ API working correctly!"
    echo ""
    echo "Found:"
    echo "  - $ALBUM_COUNT albums (of $TOTAL_ALBUMS total)"
    echo "  - $SONG_COUNT songs in first album"
    echo ""
    echo "The tool should work. If it still shows 'no songs found':"
    echo "  - There might be an issue with the C++ code"
    echo "  - Check error messages in export output"
    echo "  - Check if songs are being parsed correctly"
fi

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
