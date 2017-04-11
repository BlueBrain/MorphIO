#!/bin/bash
pandoc -s -S -c ./github-pandoc.css  ./README.md -o index.html
