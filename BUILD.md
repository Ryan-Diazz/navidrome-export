# navidrome-export CLI - Setup Guide

## Project Structure
```
navidrome-export/
├── src/
|   ├── main.cpp                # main application and export functionality
|   ├── config.cpp              # code related to the configuration file
|   ├── utilities.cpp           # all the utility functies used by the various files
|   └── test.cpp                # config and connection test command (mainly used during development)
├── include/          
|   └── json.hpp                # JSON parsing library (included)
├── CMakeLists.txt              # Build configuration
├── scripts/
|   ├── build.sh                # Automated build script
|   ├── test.sh                 # test script to test the config
|   └── diagnose-api.sh         # test script to read api responses
├── README.md                   # Documentation for users
└── BUILD.md                    # information for building and developing the program
```

## Install Dependencies

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libcurl4-openssl-dev
```

### Fedora/RHEL
```bash
sudo dnf install gcc-c++ cmake libcurl-devel
```

### Arch Linux
```bash
sudo pacman -S base-devel cmake curl
```

### Verify Installation
```bash
cmake --version
curl-config --version
```

## Step 2: Prepare the Project

clone/download the repository

```bash
cd navidrome-export
```

## Build the Project

### Build Binary

```bash
chmod +x scritps/build.sh

./scripts/build.sh
```

This will:
- Check dependencies
- Create `build/` directory
- Configure with CMake
- Compile the application
- Output: `build/navidrome-export`

### Build .deb
```bash
chmod +x scritps/create_deb.sh

./scripts/create_deb.sh
```

This will:
- Check dependencies
- Create `build/` directory
- Configure with CMake
- Compile the application
- create .deb package
- output: `navidrome-export_x.x.x_amd64.deb`

## Step 4: Configure Credentials

### Interactive Setup
```bash
./build/navidrome-export init
```

Prompts:
```
Enter Navidrome URL (e.g., http://192.168.1.100:4533): http://192.168.1.100:4533
Enter username: your_username
Enter password: your_password
```

Creates: `~/.config/navidrome-export/config.json` (permissions: 0600)

## Step 5: Export Your Library

### CSV Export (Recommended for Data)
```bash
./build/navidrome-export export --format csv --file my_music
# Output: my_music.csv
```

**Example CSV:**
```csv
Title,Artist,Album,Year,Genre,File Location
Bohemian Rhapsody,Queen,A Night at the Opera,1975,Rock,/music/Queen/A Night at the Opera/01 Bohemian Rhapsody.mp3
Stairway to Heaven,Led Zeppelin,Led Zeppelin IV,1971,Rock,/music/Led Zeppelin/Led Zeppelin IV/04 Stairway to Heaven.flac
```

### TXT Export (Readable Format)
```bash
./build/navidrome-export --export txt --file my_music
# Output: my_music.txt
```

**Example TXT:**
```
Queen - Bohemian Rhapsody [A Night at the Opera] (1975) [/music/Queen/A Night at the Opera/01 Bohemian Rhapsody.mp3]
Led Zeppelin - Stairway to Heaven [Led Zeppelin IV] (1971) [/music/Led Zeppelin/Led Zeppelin IV/04 Stairway to Heaven.flac]
```

### Both Formats at Once
```bash
./build/navidrome-export export --format both
# Output: navidrome_export.csv and navidrome_export.txt
```

## Step 6: (Optional) Install System-Wide

```bash
# Install to ~/.local/bin/
cd build
make install DESTDIR=$HOME/.local
```
```bash
# Then use from anywhere:
navidrome-export export csv --file my_music
```

## Command Reference

```bash
# Initialize configuration
navidrome-export init

# Export to CSV
navidrome-export export --format csv

# Export to TXT
navidrome-export export --format txt

# Export both formats
navidrome-export export --format both

# Custom output filename
navidrome-export export --format csv --file my_songs

# Help
navidrome-export help
```

## Troubleshooting

### Problem: "Config file not found"
```bash
./build/navidrome-export init
```

### Problem: "HTTP error: 401" (Unauthorized)
- Reset config
- Verify credentials work in web UI

### Problem: "Connection refused"
- Verify Navidrome URL: `curl http://192.168.1.100:4533`
- Check Navidrome service is running
- Check firewall/network access


## Use Cases

### Backup Your Library
```bash
date=$(date +%Y-%m-%d)
./build/navidrome-export export --format csv --file "music_backup_$date"
```

### Import to Spreadsheet
```bash
./build/navidrome-export export --format csv --file my_music
# Open my_music.csv in LibreOffice Calc, Excel, etc.
```

### Share Song List
```bash
./build/navidrome-export export --format txt --file songs_to_recommend
# Share songs_to_recommend.txt with friends
```

### Share to playlist creation tools
```bash
./build/navidrome-export export --format csv --file My_Library
```

## File Locations

```text
~/.config/navidrome-export/
    └── config.json              # Credentials and URL
```

## Security Notes
1. **Config file is user-readable** (0600 permissions) but can be exploited
2. **Passwords stored as hex** - use a dedicated Navidrome account if sensitive
3. **HTTPS not enforced** - use HTTP or HTTPS depending on your setup
4. **No encryption** - consider encrypting your home directory for extra security