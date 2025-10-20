#!/bin/bash

echo "Starting setup..."
echo "Installing dependencies..."

sudo apt update > /dev/null 2>&1
sudo apt install -y build-essential cmake git > /dev/null 2>&1

g++ ./prime.cpp -o prime
chmod +x ./prime

g++ ./calc.cpp -o calc
chmod +x ./calc

echo "Code compiled successfully."

mkdir -p /home/$(whoami)/cli-calculator
cp ./calc /home/$(whoami)/cli-calculator
cp ./prime /home/$(whoami)/cli-calculator

echo "Files copied to /home/$(whoami)/cli-calculator"

cd /home/$(whoami)/cli-calculator
sudo cp ./calc /usr/local/bin/
sudo chmod +x /usr/local/bin/calc

sudo cp ./prime /usr/local/bin/
sudo chmod +x /usr/local/bin/prime

echo "Calculator setup completed. You can run it using the command 'calc'."