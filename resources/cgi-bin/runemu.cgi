#!/bin/bash

echo "Content-type: text/plain"
echo ""

# To set up this hack, follow these steps:
# 1. Install Lighttpd and FastCGI by running:
#    sudo dnf install lighttpd lighttpd-fastcgi
#
# 2. Create a home directory for the Lighttpd user:
#    sudo mkdir -p /home/lighttpd
#    sudo chown lighttpd:lighttpd /home/lighttpd
#
# 3. Copy your license file (.flexlmrc) from your home directory to the Lighttpd home directory:
#    sudo cp ~/.flexlmrc /home/lighttpd/
#    sudo chown lighttpd:lighttpd /home/lighttpd/.flexlmrc
#
# 4. Create a .bashrc file for the Lighttpd user and make sure to export the XTENSA_SYSTEM 
#    environment variable to point to your Xtensa installation:
#    sudo -u lighttpd bash -c 'echo "export XTENSA_SYSTEM=/path/to/xtensa" > /home/lighttpd/.bashrc'

# Set HOME environment variable
export HOME="/home/lighttpd"

# Source .bashrc to load environment variables
source /home/lighttpd/.bashrc

# Function to decode URL-encoded strings
url_decode() {
    local encoded="$1"
    printf '%b' "${encoded//%/\\x}"
}

# Extract and decode the argument from the query string
RAW_ARG=$(echo "$QUERY_STRING" | sed -n 's/^.*arg=\([^&]*\).*$/\1/p')
ARG=$(url_decode "$RAW_ARG")

# Extract and decode the summary parameter from the query string
RAW_SUMMARY=$(echo "$QUERY_STRING" | sed -n 's/^.*summary=\([^&]*\).*$/\1/p')
SUMMARY=$(url_decode "$RAW_SUMMARY")

# Build the command with or without the --summary option
if [ "$SUMMARY" == "yes" ]; then
    stdbuf -oL xt-run --summary firmware.elf "$ARG"
else
    stdbuf -oL xt-run --version
    stdbuf -oL xt-run firmware.elf "$ARG"
fi
