# navidrome-export CLI

A lightweight C++ CLI tool to export all the songs in your Navidrome music library as CSV or TXT file.

## Features

- **Two export formats:**
  - **CSV**: Structured data (Title, Artist, Album, Year, Genre, File Location)
  - **TXT**: Human-readable format: `Artist - Title [Album] (Year) [Location]`
- **config storage** in `~/.config/navidrome-export/config.json`

## Usage

### Initialize Configuration
```bash
navidrome-export init
```

This will:
1. Create `~/.config/navidrome-export/` directory
2. Prompt for your Navidrome details
3. Save config with restricted permissions (0600)

Example prompt:
```
=== navidrome-export Configuration ===
Enter Navidrome URL (e.g., http://192.168.1.100:4533): http://192.168.1.100:4533
Enter username: myusername
Enter password: 
```
* the Password field will hide input characters.


Config saved to: ~/.config/navidrome-export/config.json

### Export to CSV
```bash
navidrome-export export --format csv
# Output: navidrome_export.csv
```

### Export to TXT (formatted strings)
```bash
navidrome-export export --format txt
# Output: navidrome_export.txt
```

### Export to both formats
```bash
navidrome-export export --format both
# Output: navidrome_export.csv and navidrome_export.txt
```

### Custom output filename
```bash
navidrome-export export --format csv --file my_music_collection
# Output: my_music_collection.csv
```

### Short options
```bash
navidrome-export export -f my_music --format both
```

### Reset Configuration
```bash
navidrome-export reset
```

This deletes the config file. You'll need to run `init` again to be able to use this program.

### Help
```bash
navidrome-export help
navidrome-export export --help
navidrome-export export -h
```

## Output Examples

### CSV Format
```csv
Title,Artist,Album,Year,Genre,File Location
Bohemian Rhapsody,Queen,A Night at the Opera,1975,Rock,/music/Queen/A Night at the Opera/Bohemian Rhapsody.mp3
Stairway to Heaven,Led Zeppelin,Led Zeppelin IV,1971,Rock,/music/Led Zeppelin/Led Zeppelin IV/Stairway to Heaven.flac
```

### TXT Format
```
Queen - Bohemian Rhapsody [A Night at the Opera] (1975) [/music/Queen/A Night at the Opera/Bohemian Rhapsody.mp3]
Led Zeppelin - Stairway to Heaven [Led Zeppelin IV] (1971) [/music/Led Zeppelin/Led Zeppelin IV/Stairway to Heaven.flac]
```

## How It Works

1. **Reads config** from `~/.config/navidrome-export/config.json`
2. **Connects to Navidrome** via Subsonic API
3. **Fetches all songs** with pagination (500 songs per request)
4. **Exports** in your chosen format(s)
5. **Handles special characters** properly (CSV escaping, URL encoding, etc.)

## Code Structure

- **Config Management**: Initialize and load credentials
- **API Communication**: CURL-based requests to Navidrome's Subsonic API
- **Data Processing**: Parse JSON responses and populate Song objects
- **Export Functions**: Format and write CSV/TXT files
- **Error Handling**: error messages unique to certain parts of the code

## Troubleshooting

### "Config file not found"
Run: `navidrome-export init`

### "HTTP error: 401"
Reconfigure the credentials by running `reset` and `init`

### "Connection refused"
Verify:
- Navidrome URL is correct and reachable
- Navidrome service is running
- No firewall blocking the connection

### "Failed to initialize CURL"
Ensure libcurl is installed and properly linked

## Security Notes
- Config file permissions are set to `0600` (user read/write only)
- Passwords are stored as hex in the config file
- Use a dedicated Navidrome user account if sensitive
